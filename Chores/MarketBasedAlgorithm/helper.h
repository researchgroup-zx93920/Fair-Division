#include <bits/stdc++.h>
using namespace std;

const char separator    = ' ';
const int nameWidth     = 11;
const int numWidth      = 11;

class Nodes {
    public:
        string type;
        int index;
        Nodes() {
            index = 0;
        }
        virtual void printType() {
            cout << type << endl;
        }
};

class ItemNodes: public Nodes {
    public:
        double price;
        int allocatedAgent;
        ItemNodes() : Nodes() {
            price = numeric_limits<double>::max();
            allocatedAgent = {};
            type = "ItemNode";
        }
        void printType() {
            cout << type << endl;
        }
};

class AgentNodes: public Nodes {
    public: 
        double bundlePrice;
        double MBB;
        vector<double> itemUtilityMap;
        vector<ItemNodes*> allocationItems;
        vector<ItemNodes*> MBBItems;

        AgentNodes() : Nodes() {
            itemUtilityMap = {};
            bundlePrice = 0;
            allocationItems = {};
            MBBItems = {};
            MBB = 1;
            type = "AgentNode";
        }

        void printType() {
            cout << type << endl;
        }

        void printAgentAllocation() {
            cout << left << setw(nameWidth) << setfill(separator) << "Agent " << index << " -> ";
            for (auto i: allocationItems) {
                cout << left << setw(nameWidth) << setfill(separator) << (*i).index;
            }
            cout << endl;
        }

        void printAgentMBB() {
            cout << left << setw(nameWidth) << setfill(separator) << "Agent " << index << " -> ";
            for (auto i: MBBItems) {
                cout << left << setw(nameWidth) << setfill(separator) << (*i).index;
            }
            cout << endl;
        }

        // double printBundlePrice() {
        //     double bundlePrice = 0;
        //     for(auto item:allocationItems) {
        //         bundlePrice+=item->price;
        //     }
        //     return bundlePrice;
        // }
};

// sample instantiation
void generateSample(int seed, string distribution_type, vector<double> parameters, vector<AgentNodes> &agents, vector<ItemNodes> &items, ofstream &sampleFile);
void populateInstance(vector<AgentNodes> &agents, vector<ItemNodes> &items, double &minBundlePrice);
void populateInstanceWithOneEach(vector<AgentNodes> &agents, vector<ItemNodes> &items);

// double comparisons
bool doubleIsEqual(double v1, double v2, double epsilon);
bool doubleIsGreaterOrEqual(double v1, double v2, double epsilon);
bool doubleIsGreater(double v1, double v2, double epsilon);

// metrics
double findMinBundlePrice(vector<AgentNodes> &agents);
double findEFMaxBundlePrice(vector<AgentNodes> &agents, vector<ItemNodes> &items, int agent=-1);
double findEFMaxPlusMinValuation(vector<AgentNodes> &agents, vector<ItemNodes> &items, int agent=-1);
double findBundleValuation(int bundleAgent, int referenceAgent, vector<AgentNodes> &agents);
double findEFMaxValuation(int bundleAgent, int referenceAgent, vector<AgentNodes> &agents);
long double findNashEFMaxWelfare(vector<AgentNodes> &agents, vector<ItemNodes> &items);
long double findNashWelfare(vector<AgentNodes> &agents, vector<ItemNodes> &items);
double findMinEnvyDiff(vector<AgentNodes> &agents);

// find agents
vector<int> findLeastSpenders( vector<AgentNodes> &agents, double minBundlePrice);
vector<int> findBigSpenders(vector<AgentNodes> &agents, vector<ItemNodes> &items, double EFMaxBundlePrice);

// price increase procedures
double computeAlpha1(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> &agents, vector<ItemNodes> &items);
double computeAlpha2(unordered_set<int> LSComponentAgents,  vector<AgentNodes> &agents, double minBundlePrice);
void updateItemPrices(unordered_set<int> LSComponentItems, vector<ItemNodes> &items, double beta);
void updateAgentBundles(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> &agents, vector<ItemNodes> &items, double beta);
void transferItem(int itemToTransfer, int transferFromAgent, int tranferToAgent, vector<AgentNodes> &agents, vector<ItemNodes> &items);

// brute checks
bool checkEF1BetweenAgents(int agentEnvying, int agentEnvied, vector<AgentNodes> &agents);
bool checkDEF1BetweenAgents(int agentEnvying, int agentEnvied, vector<AgentNodes> &agents);
bool is_PEF1_fPO(vector<AgentNodes> &agents, vector<ItemNodes> &items);
bool is_EF1_fPO(vector<AgentNodes> &agents, vector<ItemNodes> &items);
int checkMetricMonotonicityWhenSameAgentbecomesLS(string trend, int LS, unordered_map<int, long double> &valuationMap, long double metric, 
                                                    vector<AgentNodes> &agents, vector<ItemNodes> &items);
