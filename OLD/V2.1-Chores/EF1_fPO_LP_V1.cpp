#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "/opt/gurobi952/linux64/include/gurobi_c++.h"
#define EPS 0.0001f

// Description V1: Y is binary, Y constraints are dirctly added, objective is min (sum dX + sum dY), 
// we check EF1 and fPO property at the end, i.e. 
// g++ -I/opt/gurobi952/linux64/include/ -L/opt/gurobi952/linux64/lib helper.o output.o -o EF1_LP EF1_LP.cpp -lgurobi_c++ -lgurobi95
// https://support.gurobi.com/hc/en-us/community/posts/360073121211-Undefined-references-problem-when-compiling-c-test-code

using namespace std;

void printGRBvar(vector<vector<GRBVar>> &X, int r, int c);

int main() {

    int samples = 10000, iteration = 0;    // number of samples to run the code for
    string dist_type = "trivalued";         // distribution to generate valutions of agents from - set parameters below
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
        parameters = {20,50};               // [a, b] - the two different values
    else if(dist_type=="trivalued")
        parameters = {20, 50, 70};               // [a, b, c] - the two different values

    ofstream sampleFileGurobi;
    sampleFileGurobi.open("./Logs/SamplesGurobi.txt");

    // --------------------------------- SAMPLE ITERATION ---------------------------------
    while(iteration < samples) {

        cout << "Working on Sample Number " << iteration << endl;

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(2, 5);
        std::uniform_int_distribution<int> uniform_dist_item(1, 15);

        // define inputs - initialize n - agents (iterator-> i), m - items (iterator-> j)
        int n = uniform_dist_agent(rng);
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
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", "EF1_to_fPO_GurboiLog.log");
            env.start();
            GRBModel EF1_fPO_model = GRBModel(env);
            // EF1_fPO_model.set(GRB_IntParam_LogToConsole, 0);

            // add X variable
            vector<vector<GRBVar>> X(n, vector<GRBVar>(m));
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    X[i][k] = EF1_fPO_model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                }         
            }

            // add Z variable
            cout << "Adding Z";
            vector<GRBVar> Z(n);
            for(int i = 0; i < n; i++) {
                Z[i] = EF1_fPO_model.addVar(0.0, GRB_INFINITY, 0.0 , GRB_CONTINUOUS);   
            }

            // Constraint 1
            for(int k = 0; k < m; k++) {
                GRBLinExpr expr;
                for(int i = 0; i < n; i++) {
                    expr+=(X[i][k]);
                }
                EF1_fPO_model.addConstr(expr==1);
                expr.clear();
            }

            // Constraint 2
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n; j++) {
                    if(i==j) continue;

                    GRBLinExpr expr;
                    for(int k = 0; k < m; k++) {
                        expr+=( (agents[i].itemUtilityMap[k] * X[i][k]) - (agents[i].itemUtilityMap[k] * X[j][k]) );
                    }
                    EF1_fPO_model.addConstr(expr<=Z[i]);
                    expr.clear();
                }
                for(int k = 0; k < m; k++) {
                    EF1_fPO_model.addConstr( Z[i] >= (agents[i].itemUtilityMap[k] * X[i][k]) );
                }
            }

            // add auxillary variables for multiplying coefficients
            GRBVar A[n][m];
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    A[i][k] = EF1_fPO_model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
                    EF1_fPO_model.addConstr( A[i][k] == (X[i][k] * agents[i].itemUtilityMap[k]) );
                }
            }

            // constraint 3
            for(int i = 0; i < n; i++) {
                EF1_fPO_model.addGenConstrMax(Z[i], A[i], m);
            }

            // fPO variable Y
            vector<vector<GRBVar>> Y(n, vector<GRBVar>(m));
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    Y[i][k] = EF1_fPO_model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                }         
            }

            // C1
            for(int i = 0; i < n; i++) {
                GRBLinExpr expr1;
                GRBLinExpr expr2;
                for(int k = 0; k < m; k++) {
                    expr1+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                    expr2+=(agents[i].itemUtilityMap[k]*X[i][k]);
                }
                EF1_fPO_model.addConstr(expr1<=expr2);
                expr1.clear();
                expr2.clear();
            }

            // C2
            for(int k = 0; k < m; k++) {
                GRBLinExpr expr;
                for(int i = 0; i < n; i++) {
                    expr+=(Y[i][k]);
                }
                EF1_fPO_model.addConstr(expr==1);
                expr.clear();
            }

            // Objective
            GRBLinExpr obj;
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    obj+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                    obj+=(agents[i].itemUtilityMap[k]*X[i][k]);
                }
            }

            // parameters
            EF1_fPO_model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
            EF1_fPO_model.set(GRB_IntParam_PoolSolutions, 1);
            // EF1_fPO_model.set(GRB_DoubleParam_PoolGap, 1); // Limit the search space by setting a gap for the worst possible solution that will be accepted
            EF1_fPO_model.set(GRB_IntParam_PoolSearchMode, 2); // find all solutions mode
            EF1_fPO_model.set(GRB_DoubleParam_IntFeasTol, 1e-3); // set model tolerance
            // EF1_fPO_model.set(GRB_IntParam_IntegralityFocus, 1); // set Integrality Focus
            // EF1_fPO_model.set(GRB_STR_PAR_SOLFILES, "./Logs/solutions/mymodel"); // specify file to save solutions
            EF1_fPO_model.write("EF1_LP_poolsearch_c++.lp");

            // optimize and print final values
            EF1_fPO_model.setObjective(obj, GRB_MINIMIZE);
            EF1_fPO_model.optimize();

            // check if model was feasible or not
            int optimstatus = EF1_fPO_model.get(GRB_IntAttr_Status);
            if (optimstatus == GRB_INF_OR_UNBD) {
                EF1_fPO_model.set(GRB_IntParam_Presolve, 0);
                EF1_fPO_model.optimize();
                optimstatus = EF1_fPO_model.get(GRB_IntAttr_Status);
                return 1;
            }
            if (optimstatus == GRB_OPTIMAL) {
            
                // Print number of solutions stored
                int nSolutions = EF1_fPO_model.get(GRB_IntAttr_SolCount);
                cout << "Number of solutions found: " << nSolutions << endl;

                // Print all solutions
                for (int e = 0; e < nSolutions; e++) {

                    // access solution number `e`
                    EF1_fPO_model.set(GRB_IntParam_SolutionNumber, e);

                    // ----------------=--- check the solution is EF1 or not --------------------------------------
                    for(int i = 0; i < agents.size(); i++) {
                        double bundleValuationI = 0;
                        for(int k = 0; k < m; k++) {
                            bundleValuationI+=A[i][k].get(GRB_DoubleAttr_Xn);
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
                                return 0;
                            }
                        }
                    }

                    // -------------check for fPO and report difference between objectives------------------------
                    GRBEnv * env1 = 0;
                    env1 = new GRBEnv ();
                    GRBModel fPO_model = GRBModel (*env1);
                    fPO_model.set(GRB_IntParam_LogToConsole, 0);

                    // add the Y variable
                    vector<vector<GRBVar>> y(n, vector<GRBVar>(m));
                    for(int i = 0; i < n; i++) {
                        for(int k = 0; k < m; k++) {
                            y[i][k] = fPO_model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                        }         
                    }

                    // C1
                    for(int i = 0; i < n; i++) {
                        GRBLinExpr expr1;
                        double expr2 = 0;
                        for(int k = 0; k < m; k++) {
                            expr1+=(agents[i].itemUtilityMap[k]*y[i][k]);
                            expr2+=(agents[i].itemUtilityMap[k]*X[i][k].get(GRB_DoubleAttr_Xn));
                        }
                        fPO_model.addConstr(expr1<=expr2);
                        expr1.clear();
                    }

                    // C2
                    for(int k = 0; k < m; k++) {
                        GRBLinExpr expr1;
                        for(int i = 0; i < n; i++) {
                            expr1+=(y[i][k]);
                        }
                        fPO_model.addConstr(expr1==1);
                        expr1.clear();
                    }

                    // set objective
                    GRBLinExpr obj_fPO;
                    for(int i = 0; i < n; i++) {
                        for(int k = 0; k < m; k++) {
                            obj_fPO+=(agents[i].itemUtilityMap[k]*y[i][k]);
                        }
                    }

                    fPO_model.setObjective(obj_fPO, GRB_MINIMIZE);
                    fPO_model.optimize();

                    int optimstatus_fPO = fPO_model.get(GRB_IntAttr_Status);
                    if (optimstatus_fPO == GRB_INF_OR_UNBD) {
                        cout << "Model is unobounded";
                        fPO_model.set(GRB_IntParam_Presolve, 0);
                        fPO_model.optimize();
                        optimstatus_fPO = fPO_model.get(GRB_IntAttr_Status);
                        return 1;
                    }
                    if (optimstatus_fPO == GRB_OPTIMAL) {

                        double X_sum = 0;
                        double y_sum = 0;
                        for(int i = 0; i < n; i++) {
                            for(int k = 0; k < m; k++) {
                                X_sum+=(agents[i].itemUtilityMap[k]*(X[i][k].get(GRB_DoubleAttr_Xn)));
                                y_sum+=(agents[i].itemUtilityMap[k]*(y[i][k].get(GRB_DoubleAttr_X)));
                            }
                        }

                        // if(abs(X_sum - fPO_model.get(GRB_DoubleAttr_ObjVal)) < 1) {
                            // cout << "Found the optimal Solution!";
                            cout << "\n-------------------------------------------\n";
                            cout << "Printing X ():" <<  e << "/" << nSolutions << "\n";
                            printGRBvar(X, n, m);
                            cout << "Printing Y ():" <<  e << "/" << nSolutions << "\n";
                            printGRBvar(Y, n, m);
                            cout << "Printing y ():\n";
                            printGRBvar(y, n, m);

                            cout << "Comparing Objective Values: \n";
                            cout << "X objective: " << X_sum << endl;
                            cout << "Y objective: " << (EF1_fPO_model.get(GRB_DoubleAttr_PoolObjVal) - X_sum) << endl;
                            cout << "y objective: " << fPO_model.get(GRB_DoubleAttr_ObjVal) << "\n\n"; 
                            cout << "X+Y objective: " << EF1_fPO_model.get(GRB_DoubleAttr_PoolObjVal) << endl; 
                            cout << "X+y objective: " << (X_sum + y_sum) << endl; 
                        // }

                        if(abs(X_sum - fPO_model.get(GRB_DoubleAttr_ObjVal)) < 1) {
                            cout << "Found the optimal Solution!" << endl;
                            // return 1;
                        }
                        else {cout << "Optimal Not Found" << endl;}

                    }
                    else if (optimstatus_fPO == GRB_INFEASIBLE) {
                        cout << "Model Infeasible\n"; 
                    }
                    delete env1;

                }
                
            } else if (optimstatus == GRB_INFEASIBLE) {
                cout << "Infeaible model\n"; 
            }

        } catch(GRBException e) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
        } catch(...) {
            cout << "Exception during optimization" << endl;
        }

        iteration++;
    }
    return 0;
}

// helper functions

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

