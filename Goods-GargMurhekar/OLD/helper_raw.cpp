#include <bits/stdc++.h>
#include "helper_raw.h"
#define EPS 0.0000001f

bool doubleIsEqual(double v1, double v2, double epsilon) {
    if(abs(v2-v1)<epsilon)
        return true;
    else
        return false;
}

// find the minimum Bundle price
double findMinBundlePrice(vector<AgentNodes> agents) {
    double minBundlePrice = numeric_limits<double>::max();
    for(int i = 0; i < agents.size(); i++) {
        minBundlePrice = fmin(minBundlePrice, agents[i].bundlePrice);
    }
    return minBundlePrice;
}

// find all Least Spenders based on minBundle Price
vector<int> findLeastSpenders(vector<AgentNodes> agents, double minBundlePrice) {
    vector<int> leastSpenders;
    for(int i = 0; i < agents.size(); i++) {
        if(doubleIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) 
            leastSpenders.push_back(agents[i].index);
    }
    return leastSpenders;
}

// find the Big Spender or agent who has highest utility after removing highest utility item from their bundle
double findEFMaxBundlePrice(vector<AgentNodes> agents, vector<ItemNodes> items) {
    double EFMaxBundlePrice = 0;
    for(int i = 0; i < agents.size(); i++) {
        double maxItemPrice = 0;
        for(int j = 0; j < agents[i].allocationItems.size(); j++) {
            int item = agents[i].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, items[item].price);
        }
        EFMaxBundlePrice = fmax(EFMaxBundlePrice, (agents[i].bundlePrice - maxItemPrice));
    }
    return EFMaxBundlePrice;
}

// tranfer the item to the 2nd last agent from path violator
void transferItem(int itemToTransfer, int transferFromAgent, int transferToAgent, vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    cout << "----> Transferring item to Agent " << transferToAgent << endl;
    // add item to 2nd last agent
    cout << std::setprecision(13) << "Check: " << agents[transferToAgent].bundlePrice << " " << items[itemToTransfer].price <<  " " << agents[transferToAgent].bundlePrice+items[itemToTransfer].price << endl;
    agents[transferToAgent].allocationItems.push_back(&items[itemToTransfer]);
    agents[transferToAgent].bundlePrice+=items[itemToTransfer].price;
    // remove agents from path violaters bundle 
    for (auto iter = agents[transferFromAgent].allocationItems.begin(); iter != agents[transferFromAgent].allocationItems.end(); ++iter) {
        if(*iter==&items[itemToTransfer]) {
            agents[transferFromAgent].allocationItems.erase(iter);
            cout << std::setprecision(11) << "Check: " << agents[transferFromAgent].bundlePrice << " " << items[itemToTransfer].price <<  " " << agents[transferFromAgent].bundlePrice-items[itemToTransfer].price << endl;
            agents[transferFromAgent].bundlePrice-=items[itemToTransfer].price;
            break;
        }
    }
    //update allocatedAgent for the item transferred
    items[itemToTransfer].allocatedAgent = transferToAgent;
}

// compute ratio aplha1
double computeAlpha1(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> agents, vector<ItemNodes> items) {
    double alpha1 = numeric_limits<double>::max();
    for(auto& i:LSComponentAgents) {
        for(int j = 0; j < items.size(); j++) {
            if(LSComponentItems.find(j)==LSComponentItems.end())
                alpha1 = fmin(alpha1, (agents[i].MBB)*(items[j].price)/agents[i].itemUtilityMap[j]);
        }
    }
    return alpha1;
}

// compute ratio alpha2
double computeAlpha2(unordered_set<int> LSComponentAgents,  vector<AgentNodes> agents, double minBundlePrice) {
    double alpha2 = numeric_limits<double>::max();
    for(int i = 0; i < agents.size() ; i++) {
        if(doubleIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) {
            for(int h = 0; h < agents.size(); h++) {
                if( LSComponentAgents.find(h)==LSComponentAgents.end() )
                    alpha2 = fmin(alpha2, (agents[h].bundlePrice)/agents[i].bundlePrice);
            }
        }
    }
    return alpha2;
}

// update Item Prices by beta
void updateItemPrices(unordered_set<int> LSComponentItems, vector<ItemNodes> &items, double beta) {
    for(int j = 0; j < items.size(); j++) {
        if(LSComponentItems.find(j)!=LSComponentItems.end())
            items[j].price*=beta;
    }
}

// update Agent bundle prices, MBB ratio by beta and update MBB items
void updateAgentBundles(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> &agents, vector<ItemNodes> &items, double beta) {
    for(auto& i:LSComponentAgents) {
        agents[i].bundlePrice*=beta;
        agents[i].MBB = agents[i].MBB/beta;
        // for(int j = 0; j < items.size(); j++) {
        //     if(LSComponentItems.find(j)==LSComponentItems.end() && doubleIsEqual(agents[i].MBB, agents[i].itemUtilityMap[j]/items[j].price, EPS))
        //         agents[i].MBBItems.push_back(&items[j]);
        // }
    }

    // populate MBB ratio for all agents
    for(int i = 0; i < agents.size(); i++) {
        double MBB = 0;
        for(int j = 0; j < items.size(); j++) {
            MBB = fmax(MBB, agents[i].itemUtilityMap[j]/items[j].price);
        }
        agents[i].MBB = MBB;
    }

    for(int i = 0; i < agents.size(); i++) {
        agents[i].MBBItems.clear();
        for(int j = 0; j < items.size(); j++) {
            if(doubleIsEqual(agents[i].MBB, agents[i].itemUtilityMap[j]/items[j].price, EPS)) {
                agents[i].MBBItems.push_back(&items[j]);
            } 
        }
    }
}

bool is_EF1_fPO(vector<AgentNodes> agents, vector<ItemNodes> items) {
    for(int i = 0; i < agents.size(); i++) {
        for(int k = 0; k < agents.size(); k++) {

            double maxPrice = numeric_limits<double>::min();
            for(ItemNodes* item:agents[k].allocationItems) {
                maxPrice = fmax(maxPrice, item->price);
            }


            if(agents[i].bundlePrice < (agents[k].bundlePrice - maxPrice) && doubleIsEqual(agents[i].bundlePrice, agents[k].bundlePrice-maxPrice, EPS)==false )
                return false;

        }
    }
    
    return true;
}
