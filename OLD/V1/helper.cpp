#include <bits/stdc++.h>
#include "helper.h"
#define EPS 0.0001f

bool floatIsEqual(float v1, float v2, float epsilon) {
    if(abs(v2-v1)<epsilon)
        return true;
    else
        return false;
}

// find the minimum Bundle price
float findMinBundlePrice(vector<AgentNodes> agents) {
    float minBundlePrice = numeric_limits<float>::max();
    for(int i = 0; i < agents.size(); i++) {
        minBundlePrice = fmin(minBundlePrice, agents[i].bundlePrice);
    }
    return minBundlePrice;
}

// find all Least Spenders based on minBundle Price
vector<int> findLeastSpenders(vector<AgentNodes> agents, float minBundlePrice) {
    vector<int> leastSpenders;
    for(int i = 0; i < agents.size(); i++) {
        if(floatIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) 
            leastSpenders.push_back(agents[i].index);
    }
    return leastSpenders;
}

float findEFMaxBundlePrice(vector<AgentNodes> agents, vector<ItemNodes> items) {
    float EFMaxBundlePrice = 0;
    for(int i = 0; i < agents.size(); i++) {
        float maxItemPrice = 0;
        for(int j = 0; j < agents[i].allocationItems.size(); j++) {
            int item = agents[i].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, items[item].price);
        }
        EFMaxBundlePrice = fmax(EFMaxBundlePrice, (agents[i].bundlePrice - maxItemPrice));
    }
    return EFMaxBundlePrice;
}