// #include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "helper.h"
#define EPS 0.00001f

// generates an example using specified seed value. sampleFile = file handler for storing the generated item-utility matrix
void generateSample(int seed, string distribution_type, vector<double> parameters, vector<AgentNodes> &agents, vector<ItemNodes> &items, ofstream &sampleFile) {

    cout << "Generating Example... " <<  endl;
    sampleFile << "Sample " << seed << endl;

    // populate item Utility map and determine price of each item
    vector<int> init_values(items.size(),0);

    //define lambda function
    auto generate = [&] (auto rng, auto rnd_gen) {
        for(int j = 0; j < items.size(); j++) {
            ItemNodes *item = new ItemNodes();
            for(int i = 0; i < agents.size(); i++) {
                if(j==0) {
                    AgentNodes *agent = new AgentNodes();
                    agents[i] = *agent;
                    agents[i].index = i;
                }
                agents[i].itemUtilityMap.push_back(0);
                int vij = ((init_values[j]==0)?(rng(rnd_gen)):(rng(rnd_gen)*200)) + init_values[j];                
                sampleFile << vij << " ";
                
                agents[i].itemUtilityMap[j] = (double) vij;
                items[j].price = fmin(items[j].price, agents[i].itemUtilityMap[j]);
            }
            sampleFile << endl;
            items[j].index = j;
        }
        sampleFile << endl;
    };
    
    if(distribution_type == "uniform") {
        // set parameters = [range_start, range_end]
        pcg32 rnd_gen(seed);
        std::uniform_int_distribution<int> rng((int) parameters[0], (int) parameters[1]);
        generate(rng, rnd_gen);
    }
    else if(distribution_type == "exponential") {
        // set parameters = [exponential_distribution_lambda]
        std::random_device rd; 
        std::mt19937 rnd_gen(rd ());
        std::exponential_distribution<> rng (parameters[0]);
        generate(rng, rnd_gen);
    }
    else if(distribution_type == "similar") {
        // set parameters = [standard_deviation_range_start, standard_deviation_range_end]
        pcg32 rnd_gen(seed);
        std::uniform_int_distribution<int> rng((int) parameters[0], (int) parameters[1]);
        for(int i = 1; i <= init_values.size(); i++) 
            init_values[i-1] = (i)*5000000;
        generate(rng, rnd_gen);
    }
    else if(distribution_type=="normal") {
        // set parameters = [mean, std]
        std::random_device rd{};
        std::mt19937 rnd_gen{rd()};
        std::normal_distribution<> rng{parameters[0], parameters[1]};
        generate(rng, rnd_gen);
    }
    else if (distribution_type=="bivalued") {
        if(parameters.size()!=2) {
            cout << "Less parameters \n";
            return;
        }
        pcg32 rnd_gen(seed);
        std::uniform_int_distribution<int> rng(0, 2);
        int a = (int) parameters[0];
        int b = (int) parameters[1];
        for(int j = 0; j < items.size(); j++) {
            ItemNodes *item = new ItemNodes();
            for(int i = 0; i < agents.size(); i++) {
                if(j==0) {
                    AgentNodes *agent = new AgentNodes();
                    agents[i] = *agent;
                    agents[i].index = i;
                }
                agents[i].itemUtilityMap.push_back(0);
                int vij = b;
                int v = rng(rnd_gen);
                if(v>0.5) {
                    vij = a; 
                }
                sampleFile << vij << " ";
                
                agents[i].itemUtilityMap[j] = (double) vij;
                items[j].price = fmin(items[j].price, agents[i].itemUtilityMap[j]);
            }
            sampleFile << endl;
            items[j].index = j;
        }
        
        // if all the items have the same disutility and if its=max(a,b), then lower down all to min(a,b)
        for(int i = 0; i < agents.size(); i++) {
            int minValue = numeric_limits<int>::max();
            for(int k = 0; k < items.size(); k++) {
                minValue = min((int)agents[i].itemUtilityMap[k], minValue);
            }
            if(minValue == max(a*10000, b*10000)) {
                for(int k = 0; k < items.size(); k++) {
                    agents[i].itemUtilityMap[k] = min(a*10000, b*10000);
                }
            }
         }
        sampleFile << endl;
    }
    else if (distribution_type=="trivalued") {
        if(parameters.size()!=3) {
            cout << "Less parameters \n";
            return;
        }
        pcg32 rnd_gen(seed);
        std::uniform_int_distribution<int> rng(0, 3);
        for(int j = 0; j < items.size(); j++) {
            ItemNodes *item = new ItemNodes();
            for(int i = 0; i < agents.size(); i++) {
                if(j==0) {
                    AgentNodes *agent = new AgentNodes();
                    agents[i] = *agent;
                    agents[i].index = i;
                }
                agents[i].itemUtilityMap.push_back(0); 
                int vij = (int) parameters[1]*10000; // if random number is 0 - a, 1 - b, 2 - c
                int v = rng(rnd_gen);
                if(v<0.5) {
                    vij = (int) parameters[0]*10000; 
                }
                else if(v>1.5) {
                    vij = (int) parameters[2]*10000; 
                }
                sampleFile << vij << " ";
                
                agents[i].itemUtilityMap[j] = (double) vij;
                items[j].price = fmin(items[j].price, agents[i].itemUtilityMap[j]);
            }
            sampleFile << endl;
            items[j].index = j;
        }
        sampleFile << endl;
    }
    return;
}

// populate MBB ratio/items, bundle price for every agent, an initial allocation and least spender's spending (pass by reference)
void populateInstance(vector<AgentNodes> &agents, vector<ItemNodes> &items, double &minBundlePrice) {
    // populate MBB ratio for all agents
    int n = agents.size();
    int m = items.size();
    for(int i = 0; i < n; i++) {
        double MBB = numeric_limits<double>::max();
        for(int j = 0; j < m; j++)
            MBB = fmin(MBB, agents[i].itemUtilityMap[j]/items[j].price);
        agents[i].MBB = MBB;
    }

    // populate MBB items, bundle price for every agent, an initial allocation and least spender's spending
    for(int j = 0; j < m; j++) {
        int allocated_flag = 0;
        for(int i = 0; i < n; i++) {
            if(doubleIsEqual(items[j].price, agents[i].itemUtilityMap[j], EPS)) {
                if(allocated_flag==0) {
                    agents[i].allocationItems.push_back(&items[j]);
                    agents[i].bundlePrice+=items[j].price;
                    items[j].allocatedAgent = i;
                }
                allocated_flag = 1;
            } 
            if(doubleIsEqual(agents[i].MBB, agents[i].itemUtilityMap[j]/items[j].price, EPS)) {
                agents[i].MBBItems.push_back(&items[j]);
            }   
            minBundlePrice = (j==m-1)?fmin(minBundlePrice, agents[i].bundlePrice):minBundlePrice;
        }
    }
}

void populateInstanceWithOneEach(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    // populate MBB ratio for all agents
    int n = agents.size();
    int m = items.size();

    // populate MBB items, bundle price for every agent, an initial allocation and least spender's spending
    for(int k = 0; k < m; k++) {
        agents[k%n].allocationItems.push_back(&items[k]);
        items[k].allocatedAgent = k%n;
    }
}
    
// output true if two doubles are approximately equal
bool doubleIsEqual(double v1, double v2, double epsilon) {
    if(abs(v2-v1)<epsilon)
        return true;
    else
        return false;
}

// if v1 >= v2. return true
bool doubleIsGreaterOrEqual(double v1, double v2, double epsilon) {
    if( (v1 > v2) || doubleIsEqual(v1, v2, epsilon)==true ) {
        return true;
    }
    return false;
}

// if v1 > v2. return true
bool doubleIsGreater(double v1, double v2, double epsilon) {
    if( (v1 > v2) && doubleIsEqual(v1, v2, epsilon)==false ) {
        return true;
    }
    return false;
}

// find the minimum Bundle price
double findMinBundlePrice(vector<AgentNodes> &agents) {
    double minBundlePrice = numeric_limits<double>::max();
    for(int i = 0; i < agents.size(); i++) {
        minBundlePrice = fmin(minBundlePrice, agents[i].bundlePrice);
    }
    return minBundlePrice;
}

// find all Least Spenders based on minBundle Price
vector<int> findLeastSpenders(vector<AgentNodes> &agents, double minBundlePrice) {
    vector<int> leastSpenders;
    for(int i = 0; i < agents.size(); i++) {
        if(doubleIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) 
            leastSpenders.push_back(agents[i].index);
    }
    return leastSpenders;
}

vector<int> findBigSpenders(vector<AgentNodes> &agents, vector<ItemNodes> &items, double EFMaxBundlePrice) {
    vector<int> bigSpenders;
    for(AgentNodes it:agents) {
        double EFPrice = findEFMaxBundlePrice(agents, items, it.index);
        if( doubleIsEqual(EFMaxBundlePrice, EFPrice, EPS)==true ) {
            bigSpenders.push_back(it.index);
        }
    }
    return bigSpenders;
}

// find Big Spender's (agent with highest utility after removing highest utility item from their bundle) EFMAx Bundle Price or EFMax Bundle Price of agent
double findEFMaxBundlePrice(vector<AgentNodes> &agents, vector<ItemNodes> &items, int agent) {
    double EFMaxBundlePrice = 0;
    // if agent is specified, find EFMax Bundle Price of agent
    if(agent!=-1) {
        double maxItemPrice = 0;
        for(int j = 0; j < agents[agent].allocationItems.size(); j++) {
            int item = agents[agent].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, items[item].price);
        }
        EFMaxBundlePrice = agents[agent].bundlePrice - maxItemPrice;
    }
    // else find EFMax Bundle Price of Big Spender
    else {
        for(int i = 0; i < agents.size(); i++) {
            double maxItemPrice = 0;
            for(int j = 0; j < agents[i].allocationItems.size(); j++) {
                int item = agents[i].allocationItems[j]->index;
                maxItemPrice = fmax(maxItemPrice, items[item].price);
            }
            EFMaxBundlePrice = fmax(EFMaxBundlePrice, (agents[i].bundlePrice - maxItemPrice));
        }
    }
    return EFMaxBundlePrice;
}

// find EFMax Valuation of an agent 
double findEFMaxValuation(int bundleAgent, int referenceAgent, vector<AgentNodes> &agents) {
    // find global if bundleAgent=-1 or refernceAgent=-1
    if(bundleAgent==-1 && referenceAgent==-1) {
        double maxEFMax = 0;
        int agent = -1;
        for(int i = 0; i < agents.size(); i++) {
            double EFMaxValuation = 0;
            double maxItemValuation = 0;
            for(int j = 0; j < agents[i].allocationItems.size(); j++) {
                int item = agents[i].allocationItems[j]->index;
                maxItemValuation = fmax(maxItemValuation, agents[i].itemUtilityMap[item]);
            }
            EFMaxValuation = findBundleValuation(i, i, agents) - maxItemValuation;
            maxEFMax = fmax(maxEFMax, EFMaxValuation);
            if(doubleIsEqual(maxEFMax, EFMaxValuation, EPS)==true) agent=i;
        }
        cout << "Agent: " << agent << " "; 
        return maxEFMax;      
    }

    double EFMaxValuation = 0;
    double maxItemValuation = 0;
    for(int j = 0; j < agents[bundleAgent].allocationItems.size(); j++) {
        int item = agents[bundleAgent].allocationItems[j]->index;
        maxItemValuation = fmax(maxItemValuation, agents[referenceAgent].itemUtilityMap[item]);
    }
    EFMaxValuation = findBundleValuation(bundleAgent, referenceAgent, agents) - maxItemValuation;
    return EFMaxValuation;
}

double findEFMaxPlusMinValuation(vector<AgentNodes> &agents, vector<ItemNodes> &items, int agent) {
    double EFMaxValuation = 0;
    // if agent is specified, find EFMax Valuation of agent
    if(agent!=-1) {
        double maxItemValuation = 0;
        double minItemValuation = (agents[agent].allocationItems.size()==0)?0:numeric_limits<double>::max();
        for(int j = 0; j < agents[agent].allocationItems.size(); j++) {
            int item = agents[agent].allocationItems[j]->index;
            maxItemValuation = fmax(maxItemValuation, agents[agent].itemUtilityMap[item]);
            minItemValuation = fmin(minItemValuation, agents[agent].itemUtilityMap[item]);
        }
        EFMaxValuation = findBundleValuation(agent, agent, agents) - maxItemValuation + minItemValuation;
    }
    return EFMaxValuation;
}

// tranfer the item to the 2nd last agent from path violator
void transferItem(int itemToTransfer, int transferFromAgent, int transferToAgent, vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    cout << "----> Transferring item " << itemToTransfer << " to Agent " << transferToAgent << endl;
    // add item to 2nd last agent
    // cout << std::setprecision(13) << "Check: " << agents[transferToAgent].bundlePrice << " " << items[itemToTransfer].price <<  " " << agents[transferToAgent].bundlePrice+items[itemToTransfer].price << endl;
    agents[transferToAgent].allocationItems.push_back(&items[itemToTransfer]);
    agents[transferToAgent].bundlePrice+=items[itemToTransfer].price;

    // remove agents from path violaters bundle 
    for (auto iter = agents[transferFromAgent].allocationItems.begin(); iter != agents[transferFromAgent].allocationItems.end(); ++iter) {
        if(*iter==&items[itemToTransfer]) {
            agents[transferFromAgent].allocationItems.erase(iter);
            // cout << std::setprecision(11) << "Check: " << agents[transferFromAgent].bundlePrice << " " << items[itemToTransfer].price <<  " " << agents[transferFromAgent].bundlePrice-items[itemToTransfer].price << endl;
            agents[transferFromAgent].bundlePrice-=items[itemToTransfer].price;
            break;
        }
    }
    //update allocatedAgent for the item transferred
    items[itemToTransfer].allocatedAgent = transferToAgent;
}

// compute ratio aplha1
double computeAlpha1(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    double alpha1 = numeric_limits<double>::min();
    for(auto& i:LSComponentAgents) {
        for(int j = 0; j < items.size(); j++) {
            if(LSComponentItems.find(j)==LSComponentItems.end())
                alpha1 = fmax(alpha1, (agents[i].MBB)*(items[j].price)/agents[i].itemUtilityMap[j]);
            // cout << alpha1 << " : " << i << endl;
        }
    }
    return alpha1;
}

// compute ratio aplha2
double computeAlpha2(unordered_set<int> LSComponentAgents,  vector<AgentNodes> &agents, double minBundlePrice) {
    double alpha2 = numeric_limits<double>::min();
    if(doubleIsEqual(minBundlePrice, 0, EPS))
        return 0.0f;
    for(int i = 0; i < agents.size() ; i++) {
        if(doubleIsEqual(agents[i].bundlePrice, minBundlePrice, EPS)) {
            for(int h = 0; h < agents.size(); h++) {
                if( LSComponentAgents.find(h)==LSComponentAgents.end() )
                    alpha2 = fmax(alpha2, (agents[h].bundlePrice)/agents[i].bundlePrice);
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
    }
    for(int i = 0; i < agents.size(); i++) {
        double MBB = numeric_limits<double>::max();
        for(int j = 0; j < items.size(); j++) {
            MBB = fmin(MBB, agents[i].itemUtilityMap[j]/items[j].price);
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

double findBundleValuation(int bundleAgent, int referenceAgent, vector<AgentNodes> &agents) {
    double valuation = 0;
    for(ItemNodes* item:agents[bundleAgent].allocationItems) {
        valuation+=agents[referenceAgent].itemUtilityMap[item->index];
    } 
    return valuation;
}

double findMinEnvyDiff(vector<AgentNodes> &agents) {
    double minEnvyDiff = numeric_limits<double>::max();
    for(int i = 0; i < agents.size(); i++) {

        // find item with max disutility for agent i
        double maxValuationItem = numeric_limits<double>::min();
        for(auto j:agents[i].allocationItems) {
            maxValuationItem = fmax(maxValuationItem, agents[i].itemUtilityMap[j->index]);
        }

        for(int k = 0; k < agents.size(); k++) {
            if(i!=k) {
                minEnvyDiff = fmin( minEnvyDiff, findBundleValuation(k, i, agents) - findBundleValuation(i, i, agents) + maxValuationItem);
            }
        }
    }
    return minEnvyDiff;
}

long double findNashEFMaxWelfare(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    long double nashWelfare = 1;
    for(int i = 0; i < agents.size(); i++) {
        double EFMaxValuation = findEFMaxValuation(i, i, agents);
        if(EFMaxValuation > 0 &&  doubleIsEqual(EFMaxValuation, 0, EPS)==false) {
            nashWelfare*=EFMaxValuation;
        }
    }
    return nashWelfare;
}

long double findNashWelfare(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    long double nashWelfare = 1;
    for(int i = 0; i < agents.size(); i++) {
        double valuation = findBundleValuation(i, i, agents);
        nashWelfare*=valuation;
    }
    return nashWelfare;
}

int checkMetricMonotonicityWhenSameAgentbecomesLS(string trend, int LS, unordered_map<int, long double> &valuationMap, long double metric, 
                                                    vector<AgentNodes> &agents, vector<ItemNodes> &items) {

    // trend takes values: "increasing", "decreasing"
    // status 1: value not present or is monotonic, modified map value
    // status 2: sample was non-monotonic and EF1
    // status 0: sample was not monotonic and not EF1
    if(trend=="increasing") {
        if(valuationMap.find(LS)==valuationMap.end()) {
            valuationMap.insert({LS, metric});
            return 1;
        } 

        long double prevValuation = valuationMap.at(LS);
        if( doubleIsGreaterOrEqual(prevValuation, metric, EPS)==false ) {
            valuationMap.at(LS) = metric;
            return 1;
        }
        else if( doubleIsGreater(prevValuation, metric, EPS) ) {
            cout << "Exited: PREV_METRIC_AFTER_LS_AGAIN_GREATER, prev: " << prevValuation << " now: " << metric << endl;
            
            if(is_EF1_fPO(agents, items)==true) {
                cout << "EF1 satisfied" << endl;
                return 2;
            }
            else {
                cout << "EF1 not satisfied." << endl;
                return 0;
            }
        }
    }
    else if(trend=="decreasing") {
        if(valuationMap.find(LS)==valuationMap.end()) {
            valuationMap.insert({LS, metric});
            return 1;
        } 

        long double prevValuation = valuationMap.at(LS);
        if( doubleIsGreaterOrEqual(prevValuation, metric, EPS)==true ) {
            valuationMap.at(LS) = metric;
            return 1;
        }
        else {
            cout << "Exited: PREV_METRIC_AFTER_LS_AGAIN_LESSER, prev: " << prevValuation << " now: " << metric << endl;
            
            if(is_EF1_fPO(agents, items)==true) {
                cout << "CHECK3: LS_TO_BS=1 & EF1=1" << endl;
                return 2;
            }
            else {
                cout << "CHECK3: LS_TO_BS=1 & EF1=0" << endl;
                return 0;
            }
        }
    }
    else {
        cout << "ERR: Trend incorrectly specified." << endl;
    }
    
    return 1;
}

bool checkEF1BetweenAgents(int agentEnvying, int agentEnvied, vector<AgentNodes> &agents) {
    // returns true if agentEnvying EF1 envies agentEnvied

    double maxValuation = numeric_limits<double>::min();
    for(ItemNodes* item:agents[agentEnvying].allocationItems) {
        maxValuation = fmax(maxValuation, agents[agentEnvying].itemUtilityMap[item->index]);
    }

    if(findBundleValuation(agentEnvying, agentEnvying, agents) - maxValuation > findBundleValuation(agentEnvied, agentEnvying, agents) 
        && doubleIsEqual(findBundleValuation(agentEnvying, agentEnvying, agents) - maxValuation, findBundleValuation(agentEnvied, agentEnvying, agents), EPS)==false ) {
        // cout << "Failed for agent " <<  i << "-" << findBundleValuation(i, i, agents)-maxValuation << " and agent " << k  << "-" << findBundleValuation(k, i, agents) << endl;
        return true;
    }

    return false;
}

bool checkDEF1BetweenAgents(int agentEnvying, int agentEnvied, vector<AgentNodes> &agents) {
    // returns true if agentEnvying EF1 envies agentEnvied

    double maxValuation = numeric_limits<double>::min();
    for(ItemNodes* item:agents[agentEnvying].allocationItems) {
        maxValuation = fmax(maxValuation, agents[agentEnvying].itemUtilityMap[item->index]);
    }

    if(findBundleValuation(agentEnvying, agentEnvying, agents) - maxValuation > findBundleValuation(agentEnvied, agentEnvying, agents) + maxValuation
        && doubleIsEqual(findBundleValuation(agentEnvying, agentEnvying, agents) - maxValuation, findBundleValuation(agentEnvied, agentEnvying, agents) + maxValuation, EPS)==false ) {
        // cout << "Failed for agent " <<  i << "-" << findBundleValuation(i, i, agents)-maxValuation << " and agent " << k  << "-" << findBundleValuation(k, i, agents) << endl;
        return true;
    }

    return false;
}

// check if the pEF1 condition is satisfied for an allocation
bool is_PEF1_fPO(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
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

// check if the EF1 condition is satisfied for an allocation
bool is_EF1_fPO(vector<AgentNodes> &agents, vector<ItemNodes> &items) {

    for(int i = 0; i < agents.size(); i++) {
        for(int k = 0; k < agents.size(); k++) {

            double maxValuation = numeric_limits<double>::min();
            for(ItemNodes* item:agents[i].allocationItems) {
                maxValuation = fmax(maxValuation, agents[i].itemUtilityMap[item->index]);
            }

            if(findBundleValuation(i, i, agents) - maxValuation > findBundleValuation(k, i, agents) && doubleIsEqual(findBundleValuation(i, i, agents) - maxValuation, findBundleValuation(k, i, agents), EPS)==false ) {
                cout << "Failed for agent " <<  i << "-" << findBundleValuation(i, i, agents)-maxValuation << " and agent " << k  << "-" << findBundleValuation(k, i, agents) << endl;
                return false;
            }

        }
    }
    
    return true;
}


