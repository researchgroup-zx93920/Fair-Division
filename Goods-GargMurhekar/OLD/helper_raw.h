#include <bits/stdc++.h>
using namespace std;

const char separator    = ' ';
const int nameWidth     = 6;
const int numWidth      = 6;

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
            price = 0;
            allocatedAgent = {};
            type = "ItemNode";
        }
        void printType() {
            cout << type << endl;
        }
};
class AgentNodes: public Nodes {
    public: 
        vector<double> itemUtilityMap;
        double bundlePrice;
        double MBB;
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
        
};

bool doubleIsEqual(double v1, double v2, double epsilon);

double findMinBundlePrice(vector<AgentNodes> agents);

vector<int> findLeastSpenders( vector<AgentNodes> agents, double minBundlePrice);

double findEFMaxBundlePrice(vector<AgentNodes> agents, vector<ItemNodes> items);

double computeAlpha1(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> agents, vector<ItemNodes> items);

double computeAlpha2(unordered_set<int> LSComponentAgents,  vector<AgentNodes> agents, double minBundlePrice);

void updateItemPrices(unordered_set<int> LSComponentItems, vector<ItemNodes> &items, double beta);

void updateAgentBundles(unordered_set<int> LSComponentAgents, unordered_set<int> LSComponentItems, vector<AgentNodes> &agents, vector<ItemNodes> &items, double beta);

void transferItem(int itemToTransfer, int transferFromAgent, int tranferToAgent, vector<AgentNodes> &agents, vector<ItemNodes> &items);

bool is_EF1_fPO(vector<AgentNodes> agents, vector<ItemNodes> items);