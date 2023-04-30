#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "fileLogger.h"
#define EPS 0.0001f

// ----------------------- Description: ---------------------
// Check if using the minimum ration of dk(c)/di(c) if exchaging this c* leads to minimization of Nash Welfare

using namespace std;

int main() {

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
        std::uniform_int_distribution<int> uniform_dist_item(20, 30);

        // define inputs - initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n = uniform_dist_agent(rng);
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

        while(true) {
            // remove all existing ency intially
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


        while(true) {

            double MNW = findNashWelfare(agents, items);
            double tempMNW = MNW;
            
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < n; k++) {

                    tempMNW = findNashWelfare(agents, items);
                    cout << "Nash Welfare is :" << tempMNW << endl;
                    
                    // check if agent i EF1 envies agent k
                    if(checkEF1BetweenAgents(i, k, agents)==true) {

                        cout << "Agent " << i << " EF1 envies Agent " << k << endl;

                        // // if agents k envies agent i, swap bundles and skip steps after condition
                        if(findBundleValuation(i, k, agents) < findBundleValuation(k, k, agents)) {
                            cout << "Agent " << k << " envies Agent " << i << endl;
                        }

                    }
                    else {
                        // else move to the next loop since there is no envy
                        continue;
                    }

                    // if only i envies j, find the c* = argmin dk(c*)/di(c*)
                    int cX = 0;
                    double minRatio = numeric_limits<double>::max();
                    for(auto c: agents[i].allocationItems) {
                        double ratio = (agents[k].itemUtilityMap[c->index] / agents[i].itemUtilityMap[c->index]);

                        //  if ratio of current item is less than existing minimum, reassign
                        if(ratio < minRatio && doubleIsEqual(ratio, minRatio, EPS)==false) {
                            cX = c->index;
                            minRatio = ratio;
                        }
                    }

                    // once c* is found, transfer c* from i's bundle to j's bundle
                    transferItem(cX, i, k, agents, items);
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
        


        iteration++;
        
    }

    return 0;
}