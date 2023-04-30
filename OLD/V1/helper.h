#include <bits/stdc++.h>
using namespace std;

struct ItemNodes 
{
    int index;
    string type = "Item";
    float price = 0;
    int allocatedAgent;
    // AgentNodes* MBBAgents;
};
struct AgentNodes
{
    int index;
    string type = "Agent";
    vector<float> itemUtilityMap;
    float bundlePrice = 0;
    float MBB = 1;
    vector<ItemNodes*> allocationItems;
    vector<ItemNodes*> MBBItems;

};

bool floatIsEqual(float v1, float v2, float epsilon);

float findMinBundlePrice(vector<AgentNodes> agents);

vector<int> findLeastSpenders( vector<AgentNodes> agents, float minBundlePrice);

float findEFMaxBundlePrice(vector<AgentNodes> agents, vector<ItemNodes> items);

