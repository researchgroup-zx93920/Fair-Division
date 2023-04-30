// #include <bits/stdc++.h>
#include "output.h"
#define EPS 0.0001f
using namespace std;

int main() 
{
    int samples = 5, iteration = 0;
    while(iteration < samples) {

        // initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n, m;
        string fileName = "utilities" + to_string(iteration) + ".txt";
        ifstream agentUtilities(fileName);
        agentUtilities >> n >> m;
        vector<AgentNodes> agents(n);
        vector<ItemNodes> items(m);

        // populate item Utility map and determine price of each item
        for(int j = 0; j < m; j++) {
            float itemPrice = 0;
            for(int i = 0; i < n; i++) {
                agents[i].index = i;
                agents[i].itemUtilityMap.push_back(0);
                agentUtilities >> agents[i].itemUtilityMap[j];
                itemPrice = fmax(itemPrice, agents[i].itemUtilityMap[j]);
            }
            items[j].price = itemPrice;
            items[j].index = j;
        }

        // populate MBB ratio, MBB items, bundle price for every agent, an initial allocation and least spender's spending
        float minBundlePrice = numeric_limits<float>::max();
        for(int j = 0; j < m; j++) {
            int allocated_flag = 0;
            for(int i = 0; i < n; i++) {
                if(floatIsEqual(items[j].price, agents[i].itemUtilityMap[j], EPS)) {
                    agents[i].MBBItems.push_back(&items[j]);
                    if(allocated_flag==0) {
                        agents[i].allocationItems.push_back(&items[j]);
                        agents[i].bundlePrice+=items[j].price;
                        items[j].allocatedAgent = i;
                    }
                    allocated_flag = 1;
                }    
                minBundlePrice = (j==m-1)?fmin(minBundlePrice, agents[i].bundlePrice):minBundlePrice;
            }
        }

        // print Least Spenders given minimum bundle price
        vector<int> leastSpenders = findLeastSpenders(agents, minBundlePrice);
        cout << "Least Spenders" << " -> ";
        printIntVector(leastSpenders);

        // print inital allocation
        printAgentAllocationMBB(agents);
        printRevisedPrices(items);
        cout << endl;

        // estimate EFMaxBundlePrice
        float EFMaxBundlePrice = findEFMaxBundlePrice(agents, items);
        cout << "Least Spenders Bundle Price: " << minBundlePrice << endl;
        cout << "Big Spender EFMax Bundle Price: " << EFMaxBundlePrice << endl;


        // 1.-> Do BFS with Least Spender as source to find path violator----------------------------------------------------------------------------------------1.
        while( (minBundlePrice < EFMaxBundlePrice) && floatIsEqual(minBundlePrice, EFMaxBundlePrice, EPS)==false ) {

            cout << "----> Allocation not currently pEF1" << endl;

            // 2.-> finding alternating paths from LS to path violater --------------------------------------------------------2.
            
            int path_found = 1; //denotes if path was found in the coming step or not
            unordered_set<int> leastSpenderComponentAgents, leastSpenderComponentItems; 
            while(1) {
                queue<pair<int, string>> q;
                vector<int> visitedAgent(n,0), visitedItem(m,0);
                vector<int> predAgentToItem(m,-1), predItemToAgent(n,0); //predAgent = preceding agent to an item
                // int alternate_flag = 0; //denoted whether to find an MBB item or aloctaed agent

                // revise Least Spenders if path was found or exchange occured
                if(path_found) {
                    minBundlePrice = findMinBundlePrice(agents);
                    cout << "Least Spenders Bundle Price: " << minBundlePrice << endl;
                    leastSpenders = findLeastSpenders(agents, minBundlePrice);
                    cout << "Least Spenders -> ";
                    printIntVector(leastSpenders);
                    leastSpenderComponentAgents.clear();
                    leastSpenderComponentItems.clear();
                }
                // else if no path was found but there exists multiple lease spenders move to next LS
                else {
                    leastSpenders.erase(leastSpenders.begin());
                }

                int LS = leastSpenders[0];
                int pathViolater = -1, itemViolater = -1;
                q.push({LS,"Agent"});
                visitedAgent[LS] = 1;
                leastSpenderComponentAgents.insert(LS);

                // 3. -> while there exists something to explore-----------------------------------3.
                while(!q.empty()) {
                    // alternate floag = 0 -> search for MBB items
                    if(q.front().second=="Agent") {
                        int temp = q.front().first;
                        q.pop();
                        for(ItemNodes* item:agents[temp].MBBItems) {
                            int j = item->index;
                            if(visitedItem[j]==0 && items[j].allocatedAgent!=temp) {
                                predAgentToItem[j] = temp;
                                q.push({j,"Item"});
                                visitedItem[j] = 1;
                                leastSpenderComponentItems.insert(j);

                            }
                        }
                        // alternate_flag = 1;
                    }
                    // alternate floag = 1 -> search for agent to which item is allocated
                    else if(q.front().second=="Item") {
                        int temp = q.front().first;
                        q.pop();
                        int i = items[temp].allocatedAgent;
                        if(visitedAgent[i]==0) {
                            predItemToAgent[i] = temp;
                            q.push({i,"Agent"});
                            visitedAgent[i] = 1;
                            leastSpenderComponentAgents.insert(i);
                            if( minBundlePrice < (agents[i].bundlePrice - items[temp].price) ) {
                                cout << "----> Path Violator Found" << endl;
                                cout << "Path Violater -> Agent - " << i << "; Item - " << temp << endl; 
                                pathViolater = i;
                                itemViolater = temp;
                                leastSpenderComponentItems.insert(temp);
                                break;
                            }
                        }
                        
                        // alternate_flag = 0;
                    }
                }
                // ------------------------------------------------------------------------------- 3.

                // transfer item to pred[itemViolater] from pathViolater and update bundle prices if a path violater is found
                if(pathViolater!=-1) {
                    cout << "----> Transferring item to Agent " << predAgentToItem[itemViolater] << endl;
                    // add item to 2nd last agent
                    agents[predAgentToItem[itemViolater]].allocationItems.push_back(&items[itemViolater]);
                    agents[predAgentToItem[itemViolater]].bundlePrice+=items[itemViolater].price;
                    // remove items from path violaters bundle 
                    for (auto iter = agents[pathViolater].allocationItems.begin(); iter != agents[pathViolater].allocationItems.end(); ++iter) {
                        if(*iter==&items[itemViolater]) {
                            agents[pathViolater].allocationItems.erase(iter);
                            agents[pathViolater].bundlePrice-=items[itemViolater].price;
                            break;
                        }
                    }
                     
                    //update allocatedAgent for the item transferred
                    items[itemViolater].allocatedAgent = predAgentToItem[itemViolater];
                    printAgentAllocationMBB(agents);
                    path_found = 1;
                }
                // check if no path found and there exists another least spender.
                else if(q.empty() && leastSpenders.size()>1) {
                    cout << "----> No alternating path from LS agent " << LS  << " -> Trying next LS" << endl; 
                    path_found = 0;
                }
                else if(q.empty()) {
                    // ------------------------------------------------------------------------------------------------------------2.

                    // estimate EFMaxBundlePrice
                    EFMaxBundlePrice = findEFMaxBundlePrice(agents, items);
                    cout << "Big Spenders EFMax Bundle Price: " << EFMaxBundlePrice << endl;
                    cout << "Least Spenders Bundle Price: " << minBundlePrice << endl;

                    // if pEF1 condition satisfied, come out of the loop and return the allocation
                    if((minBundlePrice > EFMaxBundlePrice) && floatIsEqual(minBundlePrice, EFMaxBundlePrice, EPS)==false ) {
                        break;
                    }
                    // else increase price of all items in LS component
                    else {

                        cout << "----> No alternating path from LS agent " << LS << " -> Increasing Prices" << endl;

                        // Add items allocated to least spender also in the Component
                        for(int i:leastSpenderComponentAgents) {
                            cout << "Adding Allocation items of LS " << i << endl;
                            for(ItemNodes* item: agents[i].allocationItems) {
                                leastSpenderComponentItems.insert(item->index);
                            }
                        }

                        // print the least spender component
                        cout << "LS Component: Agents -> ";
                        printIntSet(leastSpenderComponentAgents);
                        cout << "LS Component: Items -> ";
                        printIntSet(leastSpenderComponentItems);
                        float alpha1 = numeric_limits<float>::max();
                        float alpha2 = numeric_limits<float>::max();

                        // compute alpha 1, alpha 2 and beta
                        for(auto& i:leastSpenderComponentAgents) {
                            for(int j = 0; j < m; j++) {
                                if(leastSpenderComponentItems.find(j)==leastSpenderComponentItems.end())
                                    alpha1 = fmin(alpha1, (agents[i].MBB)*(items[j].price)/agents[i].itemUtilityMap[j]);
                            }
                        }
                        // for(auto& i:bundlePriceToAgentMap.at(minBundlePrice)) {
                        for(int i = 0; (i < n) ; i++) {
                            if(floatIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) {
                                for(int h = 0; h < n; h++) {
                                    if( leastSpenderComponentAgents.find(h)==leastSpenderComponentAgents.end() )
                                        alpha2 = fmin(alpha2, (agents[h].bundlePrice)/agents[i].bundlePrice);
                                }
                            }
                        }
                        cout << "Alpha 1 -> " << alpha1 << "; Alpha 2 -> " << alpha2 << endl;
                        float beta = fmin(alpha1, alpha2);
                        cout << "----> Increasing Price of LS Component by beta = " << beta << endl;
                        
                        // raise the prices of all items in the Least Spender component
                        for(int j = 0; j < m; j++) {
                            if(leastSpenderComponentItems.find(j)!=leastSpenderComponentItems.end())
                                items[j].price*=beta;
                        }

                        // multiply the bundle price of each least spender agent by beta,  update MBB ratio of all least spender component agents to 1/beta
                        for(auto& i:leastSpenderComponentAgents) {
                            agents[i].bundlePrice*=beta;
                            agents[i].MBB = agents[i].MBB/beta;
                            for(int j = 0; j < m; j++) {
                                if(leastSpenderComponentItems.find(j)==leastSpenderComponentItems.end() && floatIsEqual(agents[i].MBB, agents[i].itemUtilityMap[j]/items[j].price, EPS))
                                    agents[i].MBBItems.push_back(&items[j]);
                            }
                        }

                        printRevisedPrices(items);
                    }
                    path_found = 1;
                    printAgentAllocationMBB(agents);
                }
            }
            

            //redefine minimum Price Bundle and EFMax Bundle Price
            minBundlePrice = findMinBundlePrice(agents);
            EFMaxBundlePrice = findEFMaxBundlePrice(agents, items);
            cout << "Big Spenders EFMax Bundle Price: " << EFMaxBundlePrice << endl;
            cout << "Least Spenders Bundle Price: " << minBundlePrice << endl;
        }
        // -----------------------------------------------------------------------------------------------------------------------------------------------------1.

        cout << endl << "------ Allocation is now pEF1+fPO -------" << endl << endl;
        printAgentAllocationMBB(agents);
    
    iteration++;
    }

    return 0;
}

