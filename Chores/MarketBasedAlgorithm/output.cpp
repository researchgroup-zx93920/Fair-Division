#include "output.h"
#include <sciplot/sciplot.hpp>

using namespace std;

void printUtilityMap(int n, int m, vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    cout << "Printing Sample: " << endl;
    for(int i = -1; i < n; i++) {
        if(i<0) cout << left << setw(nameWidth) << setfill(separator) << "             ";
        else cout << "Agent " << to_string(i) << " - > ";
        for(int j = 0; j < m; j++) {
            if(i<0) cout << left << setw(nameWidth) << setfill(separator) << "I"+to_string(j);
            else cout << left << setw(nameWidth) << setfill(separator) << setprecision(9) << agents[i].itemUtilityMap[j];
        }
        cout << endl;
    }
    cout << endl;
}

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

void printAgentAllocationMBB(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    cout << endl << "---------- Allocations: ----------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
		agents[i].printAgentAllocation();
	}
	cout << " ------------- MBB: --------------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
		agents[i].printAgentMBB();
		// cout << "Bundle Price of Agent " << i << " -> " << agents[i].bundlePrice << endl;
	}
    cout << " ------------- P(Xi): -------------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
        cout << left << setw(nameWidth) << setfill(separator) << to_string(i)+":[ " << setprecision(11) << agents[i].bundlePrice << " ]  ";
	}
    cout << endl;
    cout << " ------------- P(Xi - g_max): -------------- " << endl;

    for(int i = 0; i < agents.size(); i++) {
        double maxItemPrice = 0;
        for(int j = 0; j < agents[i].allocationItems.size(); j++) {
            int item = agents[i].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, items[item].price);
        }
        cout << left << setw(nameWidth) << setfill(separator) << to_string(i)+":[ " << setprecision(11) << agents[i].bundlePrice - maxItemPrice << " ]  ";
    }

    cout << endl << "----------------------------------- " << endl << endl;
}

void printAgentAllocationAndValuations(vector<AgentNodes> &agents, vector<ItemNodes> &items) {
    cout << endl << "---------- Allocations: ----------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
		agents[i].printAgentAllocation();
	}
    cout << " ------------- di(Xi): -------------- " << endl;
	for(int i = 0; i < agents.size(); i++) {
        cout << left << setw(nameWidth) << setfill(separator) << to_string(i)+":[ " << setprecision(11) << findBundleValuation(i, i, agents) << " ]  ";
	}
    cout << endl;

    cout << " ------------- di(Xi - c_max): -------------- " << endl;
    for(int i = 0; i < agents.size(); i++) {
        double maxItemPrice = 0;
        for(int j = 0; j < agents[i].allocationItems.size(); j++) {
            int item = agents[i].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, agents[i].itemUtilityMap[j]);
        }
        cout << left << setw(nameWidth) << setfill(separator) << to_string(i)+":[ " << setprecision(11) << findBundleValuation(i, i, agents) - maxItemPrice << " ]  ";
    }

    cout << endl << "----------------------------------- " << endl << endl;
}

void printRevisedPrices(vector<ItemNodes> &items) {
    cout << "------------- P(j): -------------- " << endl;
	for(int j = 0; j < items.size(); j++) {
        cout << left << setw(nameWidth) << setfill(separator) << to_string(j)+":[ " << setprecision(11) << items[j].price << " ] ";
        // cout << left << setw(nameWidth) << setfill(separator) << items[j].price;
	}
    cout << endl << "----------------------------------- " << endl << endl;
}

// generate an Excel friendly output of each iteration within a sample
void generateExcel(vector<AgentNodes> &agents, vector<ItemNodes> &items, ofstream &fileHandle) {
    fileHandle << endl << "Agent ";
    for(auto agent: agents) 
        fileHandle << agent.index << " ";

    fileHandle << endl << "Bundle-Price ";
    for(auto agent: agents)
        fileHandle << agent.bundlePrice << " ";

    fileHandle << endl << "EFMax-Price ";
    for(int i = 0; i < agents.size(); i++) {
        double maxItemPrice = 0;
        for(int j = 0; j < agents[i].allocationItems.size(); j++) {
            int item = agents[i].allocationItems[j]->index;
            maxItemPrice = fmax(maxItemPrice, items[item].price);
        }
        fileHandle << agents[i].bundlePrice - maxItemPrice << " ";
    }

    fileHandle << endl << "Disutility ";
    for(auto agent: agents)
        fileHandle << findBundleValuation(agent.index, agent.index, agents) << " ";

    fileHandle << endl << "Allocation ";
    for(auto agent: agents) {
        for(auto item: agent.allocationItems)
            fileHandle << item->index << ";";
        fileHandle << " ";
    }

    fileHandle << endl << "MBB ";
    for(auto agent: agents) {
        for(auto item: agent.MBBItems)
            fileHandle << item->index << ";";
        fileHandle << " ";
    }
    fileHandle << endl << "----------------";

}

// draw the trend for upto two metrics specificed in the arguments, plots vecA and vecB with legend labels as vecA_desc and vecB_desc respectively 
void drawVerificationCurve(int iteration, vector<double> &vecA, string vecA_desc, vector<double> vecB, string vecB_desc) {
    // source: https://sciplot.github.io/
    cout << "Printing the Graph...\n";
    if(vecA.size()<2 || vecB.size()<2) return;

    // plot the graph using these 2 y values
    cout << vecA.size() << " " << vecB.size() << endl;
    sciplot::Vec x = sciplot::linspace(1, vecA.size(), vecA.size()-1);
    sciplot::Plot2D plot;
    plot.drawCurve(x, vecA).label(vecA_desc).lineWidth(0);
    plot.drawCurve(x, vecB).label(vecB_desc).lineWidth(0);
    plot.legend().atOutsideBottom().displayHorizontal().displayExpandWidthBy(2);
    plot.xlabel("Iterations");
    plot.ylabel("Value");
    sciplot::Figure figure = {{plot}};
    sciplot::Canvas canvas = {{figure}};
    // canvas.show();
    string fileName = "./Plots/Plot_"+ vecA_desc + "_" + vecB_desc + "_" + to_string(iteration)+".pdf"; 
    canvas.save(fileName);
        
}