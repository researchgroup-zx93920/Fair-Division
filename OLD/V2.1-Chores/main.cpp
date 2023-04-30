#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "fileLogger.h"
#define EPS 0.0001f
using namespace std;

int main() 
{
    // Define Inputs 
    bool DEBUG = true;                    // DEBUG Mode ON - true / OFF - false
    int samples = 10, iteration = 0;    // number of samples to run the code for
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

    // define file paths and initial headers and settings
    fileHandler f = fileHandler();

    // -------------------------------- SAMPLE ITERATION --------------------------------------//
    while(iteration < samples) {

        (DEBUG)?(cout << "Working on Sample Number " << iteration << endl):(cout << "" << endl);

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(2, 20);
        std::uniform_int_distribution<int> uniform_dist_item(1, 50);

        // define inputs - initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n = uniform_dist_agent(rng);
        int m = uniform_dist_item(rng);
        int priceRiseStepsCount = 0;
        int transferStepsCount = 0;

        // initialize and generate the sample
        vector<AgentNodes> agents(n);
        vector<ItemNodes> items(m);
        unordered_map<int, long double> valuationMap;                    // stores LS and their corresponding metrics to track
        unordered_map<string, long double> customValuationMap;     
        unordered_set<string> checkRepeat;
        unordered_map<int, long double> afterReceivingItemValuationMap;  // stores LS and their corresponding valuations after directly receiving an item
        double minBundlePrice = numeric_limits<double>::max();

        // generate the sample
        generateSample(iteration, dist_type, parameters, agents, items, f.sampleFile);

        // populate MBB ratio/items, bundle price for every agent, an initial allocation and least spender's spending (pass by reference)
        populateInstance(agents, items, minBundlePrice);

        // find Least Spenders given minimum bundle price and estimate EFMax Bundle Price
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

        // 2.-> finding alternating paths from LS to path violater --------------------------------------------------------2.
            
        int LS = -1;
        int BS = -1;
        int prevLS = -1;
        int prevBS = -1;
        int path_found = 1; //denotes if path was found in the coming step or not
        double prevValue=-1;
        double currentValue = -1;
        unordered_set<int> leastSpenderComponentAgents, leastSpenderComponentItems; 

        while(1) {

            queue<Nodes*> q;
            vector<int> visitedAgent(n,0), visitedItem(m,0);
            vector<int> predAgentToItem(m,-1), predItemToAgent(n,0); //predAgent = preceding agent to an item
            int LSToBSAgent = -1;

            if(path_found) {
                // recompute LS and BS and check if allocation is pEF1 is the path was found or exchange occured
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

                f.logValIntoFile(f.minBundlePrice_File, minBundlePrice);
                f.logValIntoFile(f.EFMaxBundlePrice_File, EFMaxBundlePrice);
                f.minBundlePrice_vec.push_back(findBundleValuation(LS, BS, agents));
                f.EFMAxBundlePrice_vec.push_back(findEFMaxValuation(BS, BS, agents));

                if( doubleIsGreaterOrEqual(minBundlePrice, EFMaxBundlePrice, EPS) ) {
                    cout << "Allocation - EF1+fPO"; 
                    break;
                }

                // LOGS
                DEBUG?(cout << "Big Spenders EFMax Bundle Price: " << EFMaxBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "Least Spenders Bundle Price: " << minBundlePrice << endl):(cout<< "");
                DEBUG?(cout << "Least Spenders -> "):(cout<< "");
                DEBUG?(printIntVector(leastSpenders)):(printIntVector({}));
                DEBUG?(cout << "Big Spenders -> "):(cout<< "");
                DEBUG?(printIntVector(bigSpenders)):(printIntVector({}));
            }
            else {
                // else if no path was found but there exists multiple lease spenders move to next LS
                leastSpenders.erase(leastSpenders.begin());
            }   

            // LOG
            if(prevLS!=LS) {
                double minBundleValuation = findBundleValuation(LS, LS, agents);
                f.logValIntoFile(f.minEnvyDiff_File, findMinEnvyDiff(agents));
                f.logValIntoFile(f.minBundleValuation_File, minBundleValuation, LS);
                f.logValIntoFile(f.EFMaxValuation_File, findEFMaxValuation(LS, LS, agents), LS);
                f.logValIntoFile(f.EFMaxPlusMinValuation_File, findEFMaxPlusMinValuation(agents, items, LS), LS);
                f.logValIntoFile(f.minAndEFMaxBundlePriceDiff_File, (EFMaxBundlePrice - minBundlePrice));
                f.logValIntoFile(f.nashEFMaxWelfare_File, findNashEFMaxWelfare(agents, items));

                generateExcel(agents, items, f.myExcel);
                DEBUG?(cout << "Least Spenders " << LS << "'s Valuation " << minBundleValuation << endl):(cout << "");
                DEBUG?(cout << "Big Spenders " << BS << "'s Valuation " << findBundleValuation(BS, BS, agents) << endl):(cout << "");
            }
            
            // insert the metric to check for monotonicity in a map
            // long double metric = (long double) (findBundleValuation(LS, BS, agents));
            // if(valuationMap.find(LS)==valuationMap.end()) {
            //     valuationMap.insert({LS, metric});
            // } 
            // else {
            //     long double prevValuation = valuationMap.at(LS);
            //     if( doubleIsGreaterOrEqual(prevValuation, metric, EPS)==false ) {
            //         // if previous value of metric was less than current, update it 
            //         valuationMap.at(LS) = metric;
            //     }
            //     else if( doubleIsGreater(prevValuation, metric, EPS) && prevLS!=LS) {
            //         // else if the metric value has strictly decreased from it previous value when LS was least spender, then exit
            //         cout << "Exited: PREV_METRIC_AFTER_LS_AGAIN_GREATER, prev: " << prevValuation << " now: " << metric << endl;
            //         goto GOTO_EXIT;
            //         // if(is_EF1_fPO(agents, items)==true) {
            //         //     cout << "CHECK1: LS_TO_BS=1 & EF1=1" << endl;
            //         //     // goto GOTO_NEXT;
            //         // }
            //         // else {
            //         //     cout << "CHECK1: LS_TO_BS=1 & EF1=0" << endl;
            //         //     // goto GOTO_EXIT;
            //         // }
            //     }
            // }

            // check if any of the past Least Spenders have become the Big Spenders
            // for(unordered_map<int, long double>::iterator it = valuationMap.begin(); it!=valuationMap.end(); it++) {
            //     if( doubleIsEqual(EFMaxBundlePrice, findEFMaxBundlePrice(agents, items, it->first), EPS)==true ) {
            //         cout << "Exited: PREV_LS_BECOMES_BS Agent " << it->first << " becomes Big Spender. Bbundle price: " << findEFMaxBundlePrice(agents, items, it->first) << endl;
            //         LSToBSAgent = it->first;

            //         if(is_EF1_fPO(agents, items)==false) cout << "EF1 condition not satisfied" << endl;
            //         else cout << "EF1 satisfied" << endl;
            //     }
            // }

            // initialize BFS params
            int pathViolater = -1, itemViolater = -1;
            Nodes* node = &agents[LS];
            q.push(node);
            visitedAgent[LS] = 1;
            leastSpenderComponentAgents.insert(LS);

            // 3. -> while there exists something to explore-----------------------------------3.
            while(!q.empty()) {
                if(q.front()->type=="AgentNode") {
                    AgentNodes* temp = dynamic_cast<AgentNodes*>(q.front());
                    q.pop();
                    for(ItemNodes* item:temp->MBBItems) {
                        int j = item->index;
                        if(visitedItem[j]==0 && (item->allocatedAgent)!=(temp->index)) {
                            predAgentToItem[j] = temp->index;
                            q.push(item);
                            visitedItem[j] = 1;
                            leastSpenderComponentItems.insert(j);
                        }
                    }
                }
                else if(q.front()->type=="ItemNode") {
                    ItemNodes* item = dynamic_cast<ItemNodes*>(q.front());
                    q.pop();
                    int i = item->allocatedAgent;
                    if(visitedAgent[i]==0) {
                        predItemToAgent[i] = item->index;
                        q.push(&agents[i]);
                        visitedAgent[i] = 1;
                        leastSpenderComponentAgents.insert(i);
                        if( path_found==1 && doubleIsGreaterOrEqual(minBundlePrice, (agents[i].bundlePrice - item->price), EPS)==false ) {
                            cout << "----> Path Violator Found" << endl;
                            DEBUG?(cout << "Path Violater -> Agent - " << i << "; Item - " << item->index << endl):(cout << "");
                            // DEBUG?(cout << "Prev Item to Agent prev " << predItemToAgent[predAgentToItem[itemViolater]] << endl):(cout << "");
                            pathViolater = i;
                            itemViolater = item->index;
                            leastSpenderComponentItems.insert(item->index);
                            break;
                        }
                    }
                }
            }
            // ------------------------------------------------------------------------------- 3.

            if(prevLS!=LS && prevLS!=-1) {
                string id = to_string(LS) + "-" + to_string(BS);
                if(customValuationMap.find(id)==customValuationMap.end()) {
                    customValuationMap.insert({id, 0});
                }
                else {
                    if(customValuationMap[id]<0.0000) {
                        cout << "Bad" << endl;
                        cout << customValuationMap[id] << endl;
                    }
                    else {
                        cout << "Good: " << customValuationMap[id] << endl;
                    }
                    customValuationMap[id] =  0;
                }
            }
            // if(prevLS!=LS && prevLS!=-1) {
            //     // log data when any least spender repeats
            //     double LSVal = findBundleValuation(LS, LS, agents);
            //     double BSVal = findBundleValuation(BS, LS, agents);
            //     string id = to_string(LS) + "-" + to_string(BS);
            //     if(customValuationMap.find(id)==customValuationMap.end()) {
            //         customValuationMap.insert({id, 0});
            //     }
            //     else {
            //         if(doubleIsGreaterOrEqual(LSVal, customValuationMap[id][0], EPS)==false) {
            //             double LSValDiff = customValuationMap[id][0] - LSVal;
            //             double BSValDiff = customValuationMap[id][1] - BSVal;
            //             cout << "\u0394LS: " << LSValDiff << " \u0394BS: " << BSValDiff << endl;
            //             if(doubleIsGreater(BSValDiff, abs(LSValDiff), EPS)) cout <<  "Good" << endl;
            //             else cout << "BAD" << endl;
            //         }
            //         else {
            //             customValuationMap[id][0] = LSVal;
            //             customValuationMap[id][1] = BSVal;

            //         }
            //     }
            //     long double metric = (long double) (findBundleValuation(LS, LS, agents));
            //     if(valuationMap.find(LS)!=valuationMap.end()) {
            //         generateExcel(agents, items, myExcel);
            //     }
            //     else {
            //         valuationMap.insert({LS, 0});
            //     }
                // valuationMap[prevLS] = findBundleValuation(prevLS, prevLS, agents) - agents[prevLS].itemUtilityMap[agents[prevLS].allocationItems.back()->index];
                // long double metric = (long double) findEFMaxValuation(agents, items, BS) - findBundleValuation(LS, BS, agents);
                // int status = checkMetricMonotonicityWhenSameAgentbecomesLS("increasing", LS, valuationMap, metric, agents, items);
                // if(status==2) goto GOTO_NEXT;
                // else if(status==0) goto GOTO_EXIT;
            // }



            // if LS becomes BS and no path was found, check if it was EF1, if yes, continue to next sample, else exit 
            // if(LSToBSAgent!=-1 && pathViolater==-1) {
            //     if(is_EF1_fPO(agents, items)==true) {
            //         cout << "CHECK1: LS_TO_BS=1 & EF1=1" << endl;
            //         // goto GOTO_NEXT;
            //     }
            //     else {
            //         cout << "CHECK1: LS_TO_BS=1 & EF1=0" << endl;
            //         // goto GOTO_EXIT;
            //     }
            // }

            // transfer item to pred[itemViolater] from pathViolater and update bundle prices if a path violater was found
            if(pathViolater!=-1) {

                // see if the pathviolator was the new BS - If not, check EF1, else continue
                // if(LSToBSAgent!=-1 && pathViolater!=LSToBSAgent) {
                //     cout << "EXIT:LS_TO_BS - LS turned BS was not path violator" << endl;
                //     if(is_EF1_fPO(agents, items)==true) {
                //         cout << "CHECK2: LS_TO_BS=1 & EF1=1" << endl;
                //         // goto GOTO_NEXT;
                //     }
                //     else {
                //         cout << "CHECK2: LS_TO_BS=1 & EF1=0" << endl;
                //         // goto GOTO_EXIT;
                //     }
                // }
                
                // perform transfer, update bundles and graph
                if(prevLS!=-1) {
                    string id = to_string(LS) + "-" + to_string(BS);
                    // if(customValuationMap.find(id)==customValuationMap.end()) {
                    //     customValuationMap.insert({id, 0});
                    // }
                    // else {
                    //     if(customValuationMap[id]<0.0000) {
                    //         cout << "Bad" << endl;
                    //         cout << customValuationMap[id] << endl;
                    //     }
                    //     customValuationMap[id] =  0;
                    // }

                    for(auto& instanceId : customValuationMap) {
                        instanceId.second+=(agents[predAgentToItem[itemViolater]].itemUtilityMap[itemViolater]);
                        instanceId.second-=(agents[pathViolater].itemUtilityMap[itemViolater]);
                    }
                }

                transferItem(itemViolater, pathViolater, predAgentToItem[itemViolater], agents, items);

                string myid = to_string(itemViolater) + "_" + to_string(pathViolater) + "_" + to_string(predAgentToItem[itemViolater]);
                string myidc = to_string(itemViolater) + "_" + to_string(predAgentToItem[itemViolater]) + "_" + to_string(pathViolater);
                if(checkRepeat.find(myid)==checkRepeat.end()) {
                    if(checkRepeat.find(myidc)!=checkRepeat.end()) {
                        cout << "REPEAT" << endl;

                        // TRIED: // EFMaxBundlePrice - minBundlePrice // minBundlePrice // findBundleValuation(LS, LS, agents) 
                        // TRIED: // ( findEFMaxValuation(BS, LS, agents) - findBundleValuation(LS, LS, agents))
                        double val = (findEFMaxValuation(pathViolater, LS, agents) - findBundleValuation(LS, LS, agents));
                        if(currentValue!=-1 && currentValue<val && is_EF1_fPO(agents, items)==false) {
                            f.logValIntoFile(f.repeatFile, val);
                            cout << "LOOSE0";
                        }
                        f.logValIntoFile(f.repeatFile, val);
                        if(is_EF1_fPO(agents, items)==true) {
                            f.repeatFile << "EF1 ";
                        }
                        else {
                            cout << "FOOL";
                        }
                        currentValue = val;
                    }
                    checkRepeat.insert(myid);
                }
                else {
                    cout << "SAME FOUND" << endl;
                    // return 0;
                }

                // // if the LS directly receives an item, log it
                // if(predAgentToItem[itemViolater]==LS) {
                //     // generateExcel(agents, items, myExcel);
                //     long double metric = findBundleValuation(LS, LS, agents);
                //     if(afterReceivingItemValuationMap.find(LS)==afterReceivingItemValuationMap.end()) {
                //         afterReceivingItemValuationMap.insert({LS, metric});
                //     }
                //     else {
                //         long double prevValuation = afterReceivingItemValuationMap.at(LS);
                //         if(prevValuation < metric && (abs(prevValuation - metric)< EPS)==false) {
                //             valuationMap.at(LS) = metric;
                //         }
                //         // if the metric value has strictly decreased from it previous value when LS was least spender, then exit
                //         else if(prevValuation > metric && (abs(prevValuation - metric) < EPS)==false) {
                //             cout << "Exited: PREV_AFTER_RECEIVING_ITEM_PROOF not satisfied prev: " << prevValuation << " now: " << metric << endl;
                //             // return 0;
                //           }
                //     }
                // }

                transferStepsCount++;
                path_found = 1;
            }
            else if(q.empty()) {
                
                // Add items allocated to least spender also in the Component
                for(int i:leastSpenderComponentAgents) {
                    for(ItemNodes* item: agents[i].allocationItems) {
                        leastSpenderComponentItems.insert(item->index);
                    }
                }

                // compute alpha 1, alpha 2 and beta
                double alpha1 = computeAlpha1(leastSpenderComponentAgents, leastSpenderComponentItems, agents, items);
                double alpha2 = computeAlpha2(leastSpenderComponentAgents, agents, minBundlePrice);
                double beta = fmax(alpha1, 0);
                
                // raise the prices of all items in the Least Spender component
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

                path_found = 1;
                priceRiseStepsCount++;
            }
            // if(is_EF1_fPO(agents, items)==true) {
            //     f.minBundlePrice_File << "EF1" << " ";
            // }
        }
        // -------------------------------------------------------------------------------------------------------------2

        // capturing total runtime
        // auto stop = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        // duration.count()

        // LOG
        f.logValIntoFile(f.logfile, priceRiseStepsCount, transferStepsCount);
        f.logValIntoFile(f.minEnvyDiff_File, findMinEnvyDiff(agents));
        f.logValIntoFile(f.minAndEFMaxBundlePriceDiff_File, EFMaxBundlePrice - minBundlePrice);

        // Final Brute Force Check for pEF1
        if(is_PEF1_fPO(agents, items)==false || is_EF1_fPO(agents, items)==false) {
            cout << "ERROR - ALLOCATION INCORRECT" << endl;
            return 0;
        }
        else {
            cout << endl << "------ Allocation is now pEF1+fPO -------" << endl << endl;
            printAgentAllocationMBB(agents, items);
            cout << "ALLOCATION - CORRECT" << endl;
        }

        // move to the next sample
        GOTO_NEXT:
        drawVerificationCurve(iteration, f.minBundlePrice_vec, "LS Bundle Valuation", f.EFMAxBundlePrice_vec, "BS EFMax Bundle Valuation (wrt LS)");
        f.nextIteration();
        iteration++;
    }

    // closing log files
    f.closeFiles();

    GOTO_EXIT:
    system("canberra-gtk-play -f ~/Downloads/pno-cs.wav"); // play sound once the program finishes
    return 0;
}

