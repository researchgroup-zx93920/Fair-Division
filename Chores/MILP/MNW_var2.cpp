#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "fileLogger.h"
#define EPS 0.0001f

// ----------------------- Description: ---------------------
// Check if using the minimum ration of dk(c)/di(c) if exchaging this c* leads to minimization of Nash Welfare
// Aim: To prove swapping i.e. i gives something to j and j gives something to i also may not decrease the
// NMW

using namespace std;

int main() {

    int samples = 100, iteration = 0;    // number of samples to run the code for
    string dist_type = "uniform";         // distribution to generate valutions of agents from - set parameters below
    vector<double> parameters;
    if(dist_type == "uniform") 
        parameters = {1, 1000};            // [range_start, range_end]
    else if(dist_type == "exponential")
        parameters = {1};                 // [exponential_distribution_lambda]
    else if(dist_type == "similar")
        parameters = {1,5};               // [standard_deviation_range_start, standard_deviation_range_end]
    else if(dist_type=="normal")
        parameters = {5,1};               // [mean, std]
    else if(dist_type=="bivalued")
        parameters = {1,2};               // [a, b] - the two different values
    else if(dist_type=="trivalued")
        parameters = {20, 50, 70};        // [a, b, c] - the two different values
        
    ofstream sampleFile;
    sampleFile.open("./Logs/Samples.txt");

    cout << "Using " << dist_type << " type disutilities with parameters: ";
    printVector(parameters);

    // --------------------------------- ITERATING OVER SAMPLES ----------------------------------------
    while(iteration < samples) {

        cout << "Working on Sample Number " << iteration << endl;

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(2, 10);
        std::uniform_int_distribution<int> uniform_dist_item(5, 15);

        // define inputs - initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n = uniform_dist_agent(rng);
        n=2;
        int m = uniform_dist_item(rng);

        // initialize and generate the sample
        vector<AgentNodes> agents(n);
        vector<ItemNodes> items(m);

        // generate the sample
        generateSample(iteration, dist_type, parameters, agents, items, sampleFile);

        // populate instance such that each agent has atleast one item (assuming n < m)
        populateInstanceWithOneEach(agents, items);

        printUtilityMap(agents.size(), items.size(), agents, items);

        printAgentAllocationMBB(agents, items);

         // remove all existing envy intially
        while(true) {
            int count = 0;

            for(int i = 0; i < n; i++) {
                for(int k = 0; k < n; k++) {

                    // check if agent i EF1 envies agent k
                    if(findBundleValuation(k, i, agents) < findBundleValuation(i, i, agents)) {

                        // if agents k envies agent i, swap bundles and skip steps after condition
                        if(findBundleValuation(i, k, agents) < findBundleValuation(k, k, agents)) {

                            count++;

                            // if agents k envies agent i, swap bundles and skip steps after condition
                            vector<ItemNodes*> tempItemSet;
                            // save i's bundle in tempItemSet
                            copy(agents[i].allocationItems.begin(), agents[i].allocationItems.end(), back_inserter(tempItemSet));
                            agents[i].allocationItems.clear();
                            // copy k's allocation set into i's
                            copy(agents[k].allocationItems.begin(), agents[k].allocationItems.end(), back_inserter(agents[i].allocationItems));
                            // copy tempSet into k's bundle
                            agents[k].allocationItems.clear();
                            copy(tempItemSet.begin(), tempItemSet.end(), back_inserter(agents[k].allocationItems));
                            continue;
                        }
                        // else just continue to tha steps after the condition
                    }
                    else {
                        // else move to the next loop since there is no envy
                        continue;
                    }

                }
            }

            if(count==0)
                break;

        }

        // c* exchange - find c* such that exchange leads to improved MNW
        while(true) {

            double MNW = findNashWelfare(agents, items);
            double tempMNW = MNW;
            
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < n; k++) {

                    cout << i << " " << k << endl;

                    tempMNW = findNashWelfare(agents, items);
                    cout << "Nash Welfare is :" << tempMNW << endl;
                    
                    // check if agent i EF1 envies agent k
                    if(checkEF1BetweenAgents(i, k, agents)==true) {

                        cout << "Agent " << i << " EF1 envies Agent " << k << endl;

                        // if agents k envies agent i, swap bundles and skip steps after condition
                        if(findBundleValuation(i, k, agents) < findBundleValuation(k, k, agents)) {
                            cout << "Weird Agent " << k << " envies Agent " << i << endl;
                        }

                    }
                    else {
                        continue; // else move to the next loop since there is no envy
                    }

                    // if only i envies j, find the first c* that can improve MNW by swapping
                    int cI = -1;
                    int cK = -1;
                    for(auto ci: agents[i].allocationItems) {
                        for(auto ck: agents[k].allocationItems) {

                            double valuationI = findBundleValuation(i, i, agents);
                            double valuationK = findBundleValuation(k, k, agents);
                            double newValuationI = valuationI - agents[i].itemUtilityMap[ci->index] + agents[i].itemUtilityMap[ck->index];
                            double newValuationK = valuationK + agents[k].itemUtilityMap[ci->index] - agents[k].itemUtilityMap[ck->index];
                            if((valuationI * valuationK) > (newValuationI * newValuationK)) {
                                cI = ci->index;
                                cK = ck->index;
                                cout << "Found [Ci, Ck] in Xi and Xk \n";
                                break;
                            }
                            else if(doubleIsEqual((valuationI * valuationK), (newValuationI * newValuationK), EPS)) {
                                cout << "EQUAL \n";
                            }
                        }
                        if(cI>-1)
                            break;
                    }

                    // if no such c is found, then check if the allocation was EF1 or not, if not, return
                    if(cI==-1) {
                        cout << "No item found in Xi, Xj \n";
                        if(is_EF1_fPO(agents, items)==false) {
                            cout << "Not EF1 \n";
                            printAgentAllocationAndValuations(agents, items);
                            return 1;
                        }
                        continue;
                    }

                    // once c* is found, transfer c* from i's bundle to j's bundle
                    transferItem(cI, i, k, agents, items);
                    transferItem(cK, k, i, agents, items);
                    printAgentAllocationAndValuations(agents, items);
                }
            }

            if(doubleIsEqual(tempMNW, MNW, EPS)==true) {
                cout << "No Improvement.. Exiting \n";
                if(is_EF1_fPO(agents, items)==true) {
                    cout << "Allocation is EF1 \n";
                }
                break;
            }
            else if(tempMNW < MNW) {
                cout << "Some improvement.. Continuing \n";
            }
            else {
                cout << "FLAW \n";
                return 0;
            }
        }


        cout << "Next Iteration\n";
        iteration++;
        
    }

    return 0;
}