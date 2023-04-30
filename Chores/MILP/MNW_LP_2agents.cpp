#include <bits/stdc++.h>
#include <cstdio>
#include <pcg_random.hpp>
#include "output.h"
#include "/opt/gurobi952/linux64/include/gurobi_c++.h"
#define EPS 0.0001f

// g++ -I/opt/gurobi952/linux64/include/ -L/opt/gurobi952/linux64/lib helper.o output.o -o EF1_LP EF1_LP.cpp -lgurobi_c++ -lgurobi95
// https://support.gurobi.com/hc/en-us/community/posts/360073121211-Undefined-references-problem-when-compiling-c-test-code
// This code tests if the minimum NW is an EF1+fPO allocation for chores or not
using namespace std;

void printGRBvar(vector<vector<GRBVar>> &X, int r, int c);

int main(int argc, char * const argv[]) {
    
    int samples = 2, iteration = 1;    // number of test samples to run the code for, starting iteration (seed = iteration number)
    string dist_type = "bivalued";         // distribution to generate valutions of agents from - set parameters below
    vector<double> parameters;
    if(dist_type == "uniform") 
        parameters = {1, 100};            // [range_start, range_end]
    else if(dist_type == "exponential")
        parameters = {1};                 // [exponential_distribution_lambda]
    else if(dist_type == "similar")
        parameters = {1,5};               // [standard_deviation_range_start, standard_deviation_range_end]
    else if(dist_type=="normal")
        parameters = {5,1};               // [mean, std]
    else if(dist_type=="bivalued")
        parameters = {20,50};             // [a, b] - the two different values
    else if(dist_type=="trivalued")
        parameters = {20, 50, 70};        // [a, b, c] - the two different values
    string filename = argv[0];

    ofstream sampleFileGurobi;
    sampleFileGurobi.open("./Logs/" + filename + "_samples.txt");

    // --------------------------------- SAMPLE ITERATION ---------------------------------
    while(iteration < samples) {

        cout << "Working on Sample Number " << iteration << endl;

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(2, 5);
        std::uniform_int_distribution<int> uniform_dist_item(3, 5);

        // define inputs - initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n = uniform_dist_agent(rng);n=2;
        int m = uniform_dist_item(rng);
        int priceRiseStepsCount = 0;
        int transferStepsCount = 0;

        // initialize and generate the sample
        vector<AgentNodes> agents(n);
        vector<ItemNodes> items(m);

        // generate the sample
        generateSample(iteration, dist_type, parameters, agents, items, sampleFileGurobi);

        printUtilityMap(agents.size(), items.size(), agents, items);
        

        // ---------------------------------- GUROBI CODE ---------------------------------
        try {
            // Create an environment
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", filename + "_GurobiLog.log");
            env.start();
            GRBModel EF1_model = GRBModel(env);
            // EF1_model.set(GRB_IntParam_LogToConsole, 0);

            // add X variable
            vector<vector<GRBVar>> X(n, vector<GRBVar>(m));
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    X[i][k] = EF1_model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                }         
            }

            // Constraint 1 - every item is assigned to one agent
            for(int k = 0; k < m; k++) {
                GRBLinExpr expr1;
                for(int i = 0; i < n; i++) {
                    expr1+=(X[i][k]);
                }
                EF1_model.addConstr(expr1==1);
                expr1.clear();
            }

            // Constraint 2 - every agent gets at least one item
            for(int i = 0; i < n; i++) {
                GRBLinExpr expr1;
                for(int k = 0; k < m; k++) {
                    expr1+=(X[i][k]);
                }
                EF1_model.addConstr(expr1>=1);
                expr1.clear();
            }

            // parameters
            EF1_model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
            EF1_model.set(GRB_IntParam_PoolSolutions, 10);
            // EF1_model.set(GRB_DoubleParam_PoolGap, 1); // Limit the search space by setting a gap for the worst possible solution that will be accepted
            EF1_model.set(GRB_IntParam_PoolSearchMode, 2); // do a systematic search for the k-best solutions
            EF1_model.set(GRB_DoubleParam_IntFeasTol, 1e-1); // set model tolerance
            // EF1_model.set(GRB_IntParam_IntegralityFocus, 1); // set Integrality Focus
            // EF1_model.set(GRB_STR_PAR_SOLFILES, "./Logs/solutions/mymodel"); // specify file to save solutions
            EF1_model.write(filename + ".lp"); // save the model

            // set Objective: minimize Nash Welfare
            GRBLinExpr bundleValuation0;
            for(int k = 0; k < m; k++) {
                bundleValuation0+=(agents[0].itemUtilityMap[k] * X[0][k]);
            }
            GRBLinExpr bundleValuation1;
            for(int k = 0; k < m; k++) {
                bundleValuation1+=(agents[1].itemUtilityMap[k] * X[1][k]);
            }
            GRBQuadExpr obj = bundleValuation0*bundleValuation1;
            EF1_model.setObjective(obj, GRB_MINIMIZE);

            // optimize and print final values
            EF1_model.optimize();

            // check if model was feasible or not
            int optimstatus = EF1_model.get(GRB_IntAttr_Status);
            if (optimstatus == GRB_INF_OR_UNBD) {
                EF1_model.set(GRB_IntParam_Presolve, 0);
                EF1_model.optimize();
                optimstatus = EF1_model.get(GRB_IntAttr_Status);
                return 1;
            }
            if (optimstatus == GRB_OPTIMAL) {
                cout << "----------------------------------\n";
                cout << "Optimal Solution: \n";
                cout << "----------------------------------\n";

                // Print number of solutions stored
                int nSolutions = EF1_model.get(GRB_IntAttr_SolCount);
                cout << "Number of solutions found: " << nSolutions << endl;

                // Print all solutions
                for (int e = 0; e < nSolutions; e++) {

                    // access solution number `e`
                    EF1_model.set(GRB_IntParam_SolutionNumber, e);
                    
                    cout << "Printing X ():" <<  e << "/" << nSolutions << "\n";
                    printGRBvar(X, n, m);

                    // ----------------=--- check the solution is EF1 or not --------------------------------------
                    for(int i = 0; i < agents.size(); i++) {

                        double bundleValuationI = 0;
                        for(int k = 0; k < m; k++) {
                            bundleValuationI+=(X[i][k].get(GRB_DoubleAttr_Xn)*agents[i].itemUtilityMap[k]);
                        }

                        for(int j = 0; j < agents.size(); j++) {

                            double bundleValuationJ = 0;
                            for(int k = 0; k < m; k++) {
                                bundleValuationJ+=(X[j][k].get(GRB_DoubleAttr_Xn)*agents[i].itemUtilityMap[k]);
                            }

                            double maxValuation = numeric_limits<double>::min();
                            for(int k = 0; k < m; k++) {
                                maxValuation = fmax(maxValuation, X[i][k].get(GRB_DoubleAttr_Xn) * agents[i].itemUtilityMap[k]);
                            }

                            if(bundleValuationI - maxValuation > bundleValuationJ && doubleIsEqual(bundleValuationI - maxValuation, bundleValuationJ, EPS)==false ) {
                                cout << "Failed for agent " <<  i << "-" << bundleValuationI - maxValuation << " and agent " << j  << "-" << bundleValuationJ << endl;
                                // return 0;
                            }
                        }
                    }

                    // ------------------- check whether the solution is fPO or not --------------------------------
                    GRBEnv * env1 = 0;
                    env1 = new GRBEnv ();
                    GRBModel fPO_model = GRBModel (*env1);
                    // GRBEnv env1 = GRBEnv(true);
                    // env1.start();
                    // GRBModel fPO_model = GRBModel(env1);
                    fPO_model.set(GRB_IntParam_LogToConsole, 0);

                    // add the Y variable
                    vector<vector<GRBVar>> Y(n, vector<GRBVar>(m));
                    for(int i = 0; i < n; i++) {
                        for(int k = 0; k < m; k++) {
                            Y[i][k] = fPO_model.addVar(0.0, 1.0, 0.0 , GRB_CONTINUOUS);   
                        }         
                    }

                    // C1
                    for(int i = 0; i < n; i++) {
                        GRBLinExpr expr1;
                        double expr2 = 0;
                        for(int k = 0; k < m; k++) {
                            expr1+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                            expr2+=(agents[i].itemUtilityMap[k]*X[i][k].get(GRB_DoubleAttr_Xn));
                        }
                        fPO_model.addConstr(expr1<=expr2);
                        expr1.clear();
                    }

                    for(int k = 0; k < m; k++) {
                        GRBLinExpr expr1;
                        for(int i = 0; i < n; i++) {
                            expr1+=(Y[i][k]);
                        }
                        fPO_model.addConstr(expr1==1);
                        expr1.clear();
                    }

                    GRBLinExpr expr1;
                    for(int i = 0; i < n; i++) {
                        for(int k = 0; k < m; k++) {
                            expr1+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                        }
                    }

                    fPO_model.setObjective(expr1, GRB_MINIMIZE);
                    fPO_model.optimize();

                    int optimstatus1 = fPO_model.get(GRB_IntAttr_Status);
                    if (optimstatus1 == GRB_INF_OR_UNBD) {
                        cout << "Model is unobounded";
                        fPO_model.set(GRB_IntParam_Presolve, 0);
                        fPO_model.optimize();
                        optimstatus1 = fPO_model.get(GRB_IntAttr_Status);
                        return 1;
                    }
                    if (optimstatus1 == GRB_OPTIMAL) {
                        cout << "Printing Y ():" <<  e << "\n";
                        printGRBvar(Y, n, m);

                        cout << "Comparing Objective Values: \n";
                        double XobjVal1 = 0, XobjVal2 = 0;
                        double YobjVal1 = 0, YobjVal2 = 0;
                        for(int k = 0; k < m; k++) {
                            XobjVal1+=(agents[0].itemUtilityMap[k]*(X[0][k].get(GRB_DoubleAttr_Xn)));
                            YobjVal1+=(agents[0].itemUtilityMap[k]*(Y[0][k].get(GRB_DoubleAttr_X)));

                            XobjVal2+=(agents[1].itemUtilityMap[k]*(X[1][k].get(GRB_DoubleAttr_Xn)));
                            YobjVal2+=(agents[1].itemUtilityMap[k]*(Y[1][k].get(GRB_DoubleAttr_X)));
                        }
                        cout << "X objective MNW: " << XobjVal1*XobjVal2 << "\nY objective MNW: " << YobjVal1*YobjVal2 << endl; 

                        double XobjVal = 0;
                        double YobjVal = 0;
                        for(int i = 0; i < n; i++) {
                            for(int k = 0; k < m; k++) {
                                XobjVal+=(agents[i].itemUtilityMap[k]*(X[i][k].get(GRB_DoubleAttr_Xn)));
                                YobjVal+=(agents[i].itemUtilityMap[k]*(Y[i][k].get(GRB_DoubleAttr_X)));
                            }
                        }
                        cout << "X objective: " << XobjVal << "\nY objective: " << fPO_model.get(GRB_DoubleAttr_ObjVal) << " " << YobjVal << endl; 

                        if(abs(XobjVal - fPO_model.get(GRB_DoubleAttr_ObjVal))<1) {
                            cout << "Found the optimal Solution!";
                            cout << EF1_model.get(GRB_DoubleAttr_PoolObjVal) << "\n";
                            // return 1;
                        }
                    }
                    else if (optimstatus1 == GRB_INFEASIBLE) {
                        cout << "----------------------------------\n";
                        cout << "Infeaible Dept Obligation date. No portfolio combination works\n"; 
                        cout << "----------------------------------\n";
                    }

                    // cout << fPO_model.getVars() << " hi " << endl;
                    delete env1;


                }
                
                
                cout << "----------------------------------\n";

            } else if (optimstatus == GRB_INFEASIBLE) {
                cout << "----------------------------------\n";
                cout << "Infeaible Dept Obligation date. No portfolio combination works\n"; 
                cout << "----------------------------------\n";
            }

        } catch(GRBException e) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        } catch(...) {
            cout << "Exception during optimization" << endl;
        }

        vector<vector<int>> x = {{0,1,1,1},{0,0,1,1},{0,0,0,1}};
        for(int i = 0; i < x.size(); i++) {
            do {
                double val0 = 0, val1 = 0;
                for(int k = 0; k < m; k++) {
                    val0+=(agents[0].itemUtilityMap[k]*(x[i][k]));
                    val1+=(agents[1].itemUtilityMap[k]*(1-x[i][k]));
                }
                cout << x[i][0] << x[i][1] << x[i][2] << x[i][3] << " - ";
                cout << val0 << " - " << val1 << " - " << val0*val1 << endl;

            } while (next_permutation(x[i].begin(), x[i].end()));
        }

        iteration++;
    }
    return 0;
}

void printGRBvar(vector<vector<GRBVar>> &X, int r, int c) {
    for(int i = 0; i < r; i++) {
        for(int k = 0; k < c; k++) {
            try{cout << round(abs(X[i][k].get(GRB_DoubleAttr_Xn))) << " ";}
            catch(...) {cout << round(abs(X[i][k].get(GRB_DoubleAttr_X))) << " ";}       
        }
        cout << "\n";
    }
    cout << "\n";
}