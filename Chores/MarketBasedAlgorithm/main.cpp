#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "fileLogger.h"
#define EPS 0.0001f

using namespace std;

int main() 
{
    // Define Inputs              
    int samples = 1000, iteration = 0;    // number of samples to run the code for, iteration == seed
    string dist_type = "uniform";         // distribution to generate valutions of agents from - set parameters below
    vector<double> parameters;
    if(dist_type == "uniform") 
        parameters = {1, 100};            // [range_start, range_end]
    else if(dist_type == "exponential")
        parameters = {1};                 // [exponential_distribution_lambda]
    else if(dist_type == "similar")
        parameters = {1,5};               // [standard_deviation_range_start, standard_deviation_range_end]
    else if(dist_type=="normal")
        parameters = {5,1};               // [mean, std]
    else if(dist_type=="bivalued")
        parameters = {1,2};               // [a, b] - the two different values
    else if(dist_type=="trivalued")
        parameters = {20, 50, 70};        // [a, b, c] - the three different values

    // File Setting
    bool DEBUG = true;                    // DEBUG Mode ON - true / OFF - false
    fileHandler f = fileHandler();        // fileHandler class handles all logs, files in which to log a particular metric  


    // -------------------------------- SAMPLE ITERATION --------------------------------------//
    while(iteration < samples) {

        (DEBUG)?(cout << "Working on Sample Number " << iteration << endl):(cout << "" << endl);

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items - specify interval to choose values from
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(3, 15);
        std::uniform_int_distribution<int> uniform_dist_item(5, 100);

        // initialize n - agents (iterator-> i), m - items (iterator-> j) and vectors to store
        int n = uniform_dist_agent(rng);
        int m = uniform_dist_item(rng);
        vector<AgentNodes> agents(n);
        vector<ItemNodes> items(m);

        // initialize variables for log
        int priceRiseStepsCount = 0;
        int transferStepsCount = 0;
        unordered_map<int, long double> valuationMap; // stores LS and their corresponding metrics to track
        double minBundlePrice = numeric_limits<double>::max();

        // generate the sample
        generateSample(iteration, dist_type, parameters, agents, items, f.sampleFile);

        // populate MBB ratio/items, bundle price for every agent, an initial allocation and least spender's spending (pass by reference)
        populateInstance(agents, items, minBundlePrice);

        // find Least Spender agents given minimum bundle price and estimate EFMax Bundle Price and find Big Spenders
        double EFMaxBundlePrice = findEFMaxBundlePrice(agents, items, -1);
        vector<int> leastSpenders = findLeastSpenders(agents, minBundlePrice);
        vector<int> bigSpenders = findBigSpenders(agents, items, EFMaxBundlePrice);

        // LOGS
        cout << "Number of Agents: " << n << "; Number of Items: " << m << endl;
        printUtilityMap(agents.size(), items.size(), agents, items);
        f.logIteration(iteration, n, m);

        DEBUG?(cout << "Least Spenders" << " -> "):(cout << "");
        DEBUG?(printIntVector(leastSpenders)):(printIntVector({}));
        DEBUG?(printAgentAllocationMBB(agents, items)):(printIntVector({}));
        DEBUG?(printRevisedPrices(items)):(printIntVector({}));
        DEBUG?(cout << endl):(cout<< "");
        DEBUG?(cout << "Least Spenders Bundle Price: " << minBundlePrice << endl):(cout<< "");
        DEBUG?(cout << "Big Spender EFMax Bundle Price: " << EFMaxBundlePrice << endl):(cout<< "");

        // ---------------- Finding alternating paths from LS to path violater until pEF1----------------------------
            
        int LS = -1;                                                                 // placeholder for current LS
        int BS = -1;                                                                 // placeholder for current BS
        int prevLS = -1;                                                             // placeholder for previous LS
        int prevBS = -1;                                                             // placeholder for previious BS
        int path_found = 1;                                                          // denotes if path was found in the coming step or not
        unordered_set<int> leastSpenderComponentAgents, leastSpenderComponentItems;  // stores items and agents in the LS component

        // until the allocation is pEF1
        while(1) {

            queue<Nodes*> q;
            vector<int> visitedAgent(n,0), visitedItem(m,0);
            vector<int> predAgentToItem(m,-1), predItemToAgent(n,-1); // predAgent = preceding agent to an item ****check*****

            if(path_found) {
                // recompute LS and BS and check if allocation is pEF1 if the path was found or exchange occured
                minBundlePrice = findMinBundlePrice(agents);
                EFMaxBundlePrice = findEFMaxBundlePrice(agents, items, -1);
                leastSpenders = findLeastSpenders(agents, minBundlePrice);
                bigSpenders = findBigSpenders(agents, items, EFMaxBundlePrice);
                leastSpenderComponentAgents.clear();
                leastSpenderComponentItems.clear();
                prevLS = LS;
                prevBS = BS;
                LS = leastSpenders[0];
                BS = bigSpenders[0];

                // LOGS
                f.minBundlePrice_vec.push_back(findMinBundlePrice(agents)); // to be able to retrieve previous LS bundle prices
                f.EFMAxBundlePrice_vec.push_back(findEFMaxBundlePrice(agents, items));
                f.logValIntoFile(f.minBundlePrice_File, minBundlePrice);
                f.logValIntoFile(f.EFMaxBundlePrice_File, EFMaxBundlePrice);

                // LOGS
                DEBUG?(cout << "Big Spenders EFMax Bundle Price: " << EFMaxBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "Big Spenders EFMax Valuation: " << findEFMaxValuation(BS, BS, agents) << endl):(cout<< "");
                DEBUG?(cout << "Highest EFMax Valuation: " << findEFMaxValuation(-1, -1, agents) << endl):(cout<< "");
                DEBUG?(cout << "Least Spenders Bundle Price: " << minBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "Least Spenders -> "):(cout<< "");
                DEBUG?(printIntVector(leastSpenders)):(printIntVector({}));
                DEBUG?(cout << "Big Spenders -> "):(cout<< "");
                DEBUG?(printIntVector(bigSpenders)):(printIntVector({}));

                // check pEF1 condition - break if condition satisfied
                if( doubleIsGreaterOrEqual(minBundlePrice, EFMaxBundlePrice, EPS) )
                    break;
            }
            else {
                // else if no path was found but there exists multiple lease spenders move to next LS
                // only applicable when running the goods algorithm in [Murhekar, Garg 2022]
                leastSpenders.erase(leastSpenders.begin());
            }   

            // LOG these values in the file: when the LS changes 
            if(prevLS!=LS) {
                double minBundleValuation = findBundleValuation(LS, LS, agents);
                f.logValIntoFile(f.minEnvyDiff_File, findMinEnvyDiff(agents));
                f.logValIntoFile(f.minBundleValuation_File, minBundleValuation, LS);
                f.logValIntoFile(f.EFMaxValuation_File, findEFMaxValuation(LS, LS, agents), LS);
                f.logValIntoFile(f.EFMaxPlusMinValuation_File, findEFMaxPlusMinValuation(agents, items, LS), LS);
                f.logValIntoFile(f.minAndEFMaxBundlePriceDiff_File, (EFMaxBundlePrice - minBundlePrice));
                f.logValIntoFile(f.nashEFMaxWelfare_File, findNashEFMaxWelfare(agents, items));

                // log these values into excel for a better log
                generateExcel(agents, items, f.myExcel);
                DEBUG?(cout << "Least Spenders " << LS << "'s Valuation " << minBundleValuation << endl):(cout << "");
                DEBUG?(cout << "Big Spenders " << BS << "'s Valuation " << findBundleValuation(BS, BS, agents) << endl):(cout << "");
            }
            
            // ----------------------------- Boilerplate for checking monotonicity ------------------------------
            // metric (map value) : quantity you want to check monotonicity for
            // event (map key) : define an event when to check the metric again
            // In this example, I store the LS as key, so I check LS's bundle valuation every time the LS repeats
            // For more examples, check the `main.cpp` file in the old folder
            // --------------------------------------------------------------------------------------------------
            long double metric = (long double) (findBundleValuation(LS, LS, agents));
            if(valuationMap.find(LS)==valuationMap.end()) {
                // if the event has never occured, insert in the map
                valuationMap.insert({LS, metric});
            } 
            else {
                long double prevValuation = valuationMap.at(LS);
                if( doubleIsGreaterOrEqual(prevValuation, metric, EPS)==false ) {
                    // update the map value corresponding to the key if metric behaves monotonically
                    valuationMap.at(LS) = metric;
                }
                else if( doubleIsGreater(prevValuation, metric, EPS) && prevLS!=LS) {
                    // else if the metric value has moved in opposite direction, exit, or do whatever you want
                    cout << "Exited: PREV_METRIC_AFTER_LS_AGAIN_NON_MONOTONIC, prev: " << prevValuation << " now: " << metric << endl;
                    goto GOTO_EXIT;
                }
            }

            // initialize BFS params
            int pathViolater = -1, itemViolater = -1;
            Nodes* node = &agents[LS];
            q.push(node);
            visitedAgent[LS] = 1;
            leastSpenderComponentAgents.insert(LS);

            // ----------------------------------------- Perform BFS of alternating MBB and allocation edges ------------------------------------------
            while(!q.empty()) {
                if(q.front()->type=="AgentNode") {
                    AgentNodes* currNode = dynamic_cast<AgentNodes*>(q.front()); // currNode = Agent
                    q.pop();
                    for(ItemNodes* item:currNode->MBBItems) {
                        int j = item->index;
                        if(visitedItem[j]==0 && (item->allocatedAgent)!=(currNode->index)) {
                            predAgentToItem[j] = currNode->index;
                            q.push(item);
                            visitedItem[j] = 1;
                            leastSpenderComponentItems.insert(j); // adding items to the LS component (creating component simultaneously)
                        }
                    }
                }
                else if(q.front()->type=="ItemNode") {
                    ItemNodes* currNode = dynamic_cast<ItemNodes*>(q.front()); // currNode = Item
                    q.pop();
                    int i = currNode->allocatedAgent;
                    if(visitedAgent[i]==0) {
                        predItemToAgent[i] = currNode->index;
                        q.push(&agents[i]);
                        visitedAgent[i] = 1;
                        leastSpenderComponentAgents.insert(i); // adding agents to the LS component (creating  component simultaneously)
                        if( path_found==1 && doubleIsGreaterOrEqual(minBundlePrice, (agents[i].bundlePrice - currNode->price), EPS)==false ) {
                            cout << "----> Path Violator Found \n";
                            DEBUG?(cout << "Path Violater -> Agent - " << i << "; Item - " << currNode->index << endl):(cout << "");
                            pathViolater = i;
                            itemViolater = currNode->index;
                            leastSpenderComponentItems.insert(currNode->index); // adding items to the LS component (creating component simultaneously)
                            break;
                        }
                    }
                }
            }
            // ----------------------------------------------------------------------------------------------------------------------------------------
            
            if(pathViolater!=-1) {
                // transfer item to pred[itemViolater] from pathViolater and update bundle prices if a path violater was found
                transferItem(itemViolater, pathViolater, predAgentToItem[itemViolater], agents, items);
                transferStepsCount++;
                path_found = 1;
            }
            else if(q.empty()) {
                // If not path was found, decrease the prices
                
                // Add items allocated to least spender also in the Component
                for(int i:leastSpenderComponentAgents) {
                    for(ItemNodes* item: agents[i].allocationItems) {
                        leastSpenderComponentItems.insert(item->index);
                    }
                }

                // compute alpha 1, alpha 2 and beta 
                double alpha1 = computeAlpha1(leastSpenderComponentAgents, leastSpenderComponentItems, agents, items);
                double alpha2 = computeAlpha2(leastSpenderComponentAgents, agents, minBundlePrice);
                double beta = fmax(alpha1, 0); // alpha 2 in chores algorithm is not useful
                
                // decrease the prices of all items in the Least Spender component
                updateItemPrices(leastSpenderComponentItems, items, beta);

                // update bundles of LS component Agents
                updateAgentBundles(leastSpenderComponentAgents, leastSpenderComponentItems, agents, items, beta);

                // LOG
                DEBUG?(cout << "Big Spenders EFMax Bundle Price: " << EFMaxBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "Least Spenders Bundle Price: " << minBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "----> No alternating path from LS agent " << LS << " -> Decreasing Prices" << endl):(cout<< "");
                
                DEBUG?(cout << "LS Component: Agents -> "):(cout<< "");
                DEBUG?(printIntSet(leastSpenderComponentAgents)):(printIntVector({}));
                DEBUG?(cout << "LS Component: Items -> "):(cout<< "");
                DEBUG?(printIntSet(leastSpenderComponentItems)):(printIntVector({}));

                cout << "Beta value is " << beta << endl;
                DEBUG?(cout << "Alpha 1 -> " << alpha1 << "; Alpha 2 -> " << alpha2 << endl):(cout<< "");
                DEBUG?(cout << "----> Decreasing Price of LS Component by beta = " << beta << endl):(cout<< "");
                DEBUG?(printRevisedPrices(items)):(printIntVector({}));
                DEBUG?(printAgentAllocationMBB(agents, items)):(printIntVector({}));

                path_found = 1; // makes sense only in goods algorithm
                priceRiseStepsCount++;
            }
        }
        // exits when allocation is pEF1

        // capturing total runtime
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        // LOG
        f.logfile << duration.count() << " " << priceRiseStepsCount << " " << transferStepsCount;
        f.logValIntoFile(f.minEnvyDiff_File, findMinEnvyDiff(agents));
        f.logValIntoFile(f.minAndEFMaxBundlePriceDiff_File, EFMaxBundlePrice - minBundlePrice);

        // Final Brute Force Check for pEF1 and EF1
        if(is_PEF1_fPO(agents, items)==false || is_EF1_fPO(agents, items)==false) {
            cout << "\n ERROR - ALLOCATION INCORRECT \n";
            return 0;
        }
        else {
            cout << "\n\n------ Allocation is now pEF1+fPO -------\n\n";
            printAgentAllocationMBB(agents, items);
        }

        // draw the monotonicity curves for any of the metrics recorded - can draw one or two values in a graph
        // drawVerificationCurve(iteration, f.minBundlePrice_vec, "LS Bundle Valuation", f.EFMAxBundlePrice_vec, "BS EFMax Bundle Valuation (wrt LS)");
        // drawVerificationCurve(iteration, f.LSBundleValuation_vec, "LS Bundle Valuation");

        // label for signal to move to the next sample
        GOTO_NEXT:
        
        f.nextIteration();
        iteration++;
    }

    // closing log files
    f.closeFiles();

    // label for signal to exit
    GOTO_EXIT:
    system("canberra-gtk-play -f ~/Downloads/pno-cs.wav"); // play sound once the program finishes so I don't forget 
    return 0;
}

