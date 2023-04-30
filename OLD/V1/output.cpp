#include <bits/stdc++.h>
#include <fstream>
#include "output.h"
// using namespace std;

const char separator    = ' ';
const int nameWidth     = 6;
const int numWidth      = 6;

// print an int vector
void printIntVector(vector<int> v) {
    for(auto i:v)
        cout << left << setw(nameWidth) << setfill(separator) << i << ", ";
    cout << endl;
}

void printIntSet(unordered_set<int> v) {
    for(auto& i:v)
        cout << left << setw(nameWidth) << setfill(separator) << i << ", ";
    cout << endl;
}

// print current allocation items of agents
void printAgentAllocation(vector<ItemNodes*> v, int agent = -1) {
    if(agent>=0) cout << left << setw(nameWidth) << setfill(separator) << "Agent " << agent << " -> ";
    for (auto i: v) {
        cout << left << setw(nameWidth) << setfill(separator) << (*i).index;
    }
    cout << endl;
}

void printAgentMBB(vector<ItemNodes*> v, int agent = -1) {
    if(agent>=0) cout << left << setw(nameWidth) << setfill(separator) << "Agent " << agent << " -> ";
    for (auto i: v) {
        cout << left << setw(nameWidth) << setfill(separator) << (*i).index;
    }
    cout << endl;
}

void printAgentAllocationMBB(vector<AgentNodes> agents) {
    cout << endl << "---------- Allocations: ----------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
		printAgentAllocation(agents[i].allocationItems, i);
	}
	cout << " ------------- MBB: --------------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
		printAgentMBB(agents[i].MBBItems, i);
		// cout << "Bundle Price of Agent " << i << " -> " << agents[i].bundlePrice << endl;
	}
    cout << "------------- P(Xi): -------------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
        cout << left << setw(nameWidth) << setfill(separator) << setprecision(6) << to_string(i) + ":[" << agents[i].bundlePrice << " ]      ";
	}
    cout << endl << "----------------------------------- " << endl << endl;
}

void printRevisedPrices(vector<ItemNodes> items) {
    cout << "------------- P(j): -------------- " << endl;
	for(int j = 0; j < items.size(); j++) {
        cout << left << setw(nameWidth) << setfill(separator) << setprecision(6) << "  " + to_string(j) + ":[" << items[j].price << " ]      ";
        // cout << left << setw(nameWidth) << setfill(separator) << items[j].price;
	}
    cout << endl << "----------------------------------- " << endl << endl;
}

void intoLog( const std::string &text )
{
    std::ofstream log_file("log_file.txt", std::ios_base::out | std::ios_base::app );
    log_file << text << endl;
}