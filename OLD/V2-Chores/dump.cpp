// 0.

// //insert BundlePrices and in a map
// priority_queue<int> bundlePriceHeap;
// unordered_map<int, vector<AgentNodes*>> bundlePriceToAgentMap;
// for(int i = 0; i < n; i++) {
//     if(bundlePriceToAgentMap.find(agents[i].bundlePrice)!=bundlePriceToAgentMap.end())
//         bundlePriceToAgentMap.at(agents[i].bundlePrice).push_back(&agents[i]);
//     else
//         bundlePriceToAgentMap.insert({agents[i].bundlePrice,{&agents[i]}});
// }

// ---------------------------------SANITY PRINT - START ---------------------------------//
// printing the bundle agent map
// for (auto it = bundlePriceToAgentMap.cbegin(); it != bundlePriceToAgentMap.cend(); ++it) {
//     for(auto c:(*it).second)
//         std::cout << (*it).first << " -> " << c->index;
//     cout << ";  ";
// }
// cout << endl;
// ---------------------------------SANITY PRINT - END------------------------------------//


// 1. 



// void updateBundlePriceToAgentMap(unordered_map<int, vector<AgentNodes*>> &bundlePriceToAgentMap, vector<AgentNodes> agents, vector<ItemNodes> items, int agent, int item, string operation) {
//     if(operation=="add") {
//         for(auto c: bundlePriceToAgentMap.at(agents[agent].bundlePrice)) {
//             if(c==)
//         }
//     }
//     else if(operation=="remove") {

//     }
// }