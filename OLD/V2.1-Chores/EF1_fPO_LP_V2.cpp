#include <bits/stdc++.h>
#include <pcg_random.hpp>
#include "output.h"
#include "/opt/gurobi952/linux64/include/gurobi_c++.h"
#define EPS 0.00001f

// g++ -I/opt/gurobi952/linux64/include/ -L/opt/gurobi952/linux64/lib helper.o output.o -o EF1_LP EF1_LP.cpp -lgurobi_c++ -lgurobi95
// https://support.gurobi.com/hc/en-us/community/posts/360073121211-Undefined-references-problem-when-compiling-c-test-code

using namespace std;

int main() {

    int samples = 3, iteration = 2;    // number of samples to run the code for
    string dist_type = "uniform";         // distribution to generate valutions of agents from - set parameters below
    vector<double> parameters;
    if(dist_type == "uniform") 
        parameters = {1, 100};            // [range_start, range_end]
    else if(dist_type == "exponential")
        parameters = {1};                 // [exponential_distribution_lambda]
    else if(dist_type == "similar")
        parameters = {1,5};               // [standard_deviation_range_start, standard_deviation_range_end]
    else if(dist_type=="normal")
        parameters = {5,1};               // [mean, std]

    ofstream sampleFileGurobi;
    sampleFileGurobi.open("./Logs/SamplesGurobi.txt");

    // --------------------------------- SAMPLE ITERATION ---------------------------------
    while(iteration < samples) {

        cout << "Working on Sample Number " << iteration << endl;

        // Get starting timepoint
        auto start = std::chrono::high_resolution_clock::now();

        // Uniform RNG for determining number of agents and items
        pcg32 rng(iteration);
        std::uniform_int_distribution<int> uniform_dist_agent(2, 9);
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
            // Create an environmen
            GRBEnv env = GRBEnv(true);
            env.set("LogFile", "EF1_fPO_GurboiLog1.log");
            env.start();
            GRBModel model = GRBModel(env);

            // add X variable
            GRBVar X[n][m];
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    X[i][k] = model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                }         
            }

            // C1
            for(int k = 0; k < m; k++) {
                GRBLinExpr expr1;
                for(int i = 0; i < n; i++) {
                    expr1+=(X[i][k]);
                }
                model.addConstr(expr1==1);
                expr1.clear();
            }

            // Constraint 2

            // add auxillary variables for multiplying coefficients
            // GRBVar A[n][m];
            // for(int i = 0; i < n; i++) {
            //     for(int k = 0; k < m; k++) {
            //         A[i][k] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
            //         model.addConstr( A[i][k] == (X[i][k] * agents[i].itemUtilityMap[k]) );
            //     }
            // }

            // constraint 3
            // for(int i = 0; i < n; i++) {
            //     model.addGenConstrMax(Z[i], A[i], m);
            // }


            // ------------------- fPO part -----------------------
            // add the y variable
            GRBVar y[n][m];
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    y[i][k] = model.addVar(0.0, 1.0, 0.0 , GRB_BINARY);   
                }         
            }

            // add the Y variable + C6
            GRBVar Y[n][m];
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    Y[i][k] = model.addVar(0.0, 1.0, 0.0 , GRB_CONTINUOUS);   
                }         
            }

            // C2 summation y_k=1 for all i
            for(int i = 0; i < n; i++) {
                GRBLinExpr expr1;
                for(int k = 0; k < m; k++) {
                    expr1+=(y[i][k]);
                }
                model.addConstr(expr1==1);
                expr1.clear();
            }

            // C3
            for(int k = 0; k < m; k++) {
                for(int i = 0; i < n; i++) {
                    model.addConstr(y[i][k] <= X[i][k]);
                }
            }

            // C8
            for(int i = 0; i < n; i++) {
                GRBLinExpr expr1;
                GRBLinExpr expr2;
                for(int k = 0; k < m; k++) {
                    expr1+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                    expr2+=(agents[i].itemUtilityMap[k]*X[i][k]);
                }
                model.addConstr(expr1<=expr2);
                expr1.clear();
                expr2.clear();
            }

            // C7 - summation Y_i = 1 for all k
            for(int k = 0; k < m; k++) {
                GRBLinExpr expr1;
                for(int i = 0; i < n; i++) {
                    expr1+=(y[i][k]);
                }
                model.addConstr(expr1==1);
                expr1.clear();
            }

            // C5
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n; j++) {
                    if(i==j)
                        continue;
                    GRBLinExpr expr1;
                    GRBLinExpr expr2;
                    GRBLinExpr expr3;
                    for(int k = 0; k < m; k++) {
                        expr1+=(agents[i].itemUtilityMap[k]*y[i][k]);
                        expr2+=(agents[i].itemUtilityMap[k]*X[i][k]);
                        expr3+=(agents[i].itemUtilityMap[k]*X[j][k]);
                    }
                    model.addConstr(expr2 - expr1 <= expr3);
                    expr1.clear();
                    expr2.clear();
                    expr3.clear();
                }
            }

            // C9
            GRBLinExpr expr1;
            GRBLinExpr expr2;
            for(int i = 0; i < n; i++) {
                for(int k = 0; k < m; k++) {
                    expr1+=(agents[i].itemUtilityMap[k]*X[i][k]);
                    expr2+=(agents[i].itemUtilityMap[k]*Y[i][k]);
                }
            }
            model.addConstr(expr2 >= expr1);


            model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

            model.set(GRB_IntParam_PoolSolutions, 20000);

            // Limit the search space by setting a gap for the worst possible solution that will be accepted
            // model.set(GRB_DoubleParam_PoolGap, 100);

            // do a systematic search for the k-best solutions
            model.set(GRB_IntParam_PoolSearchMode, 2);

            // set model tolerance
            model.set(GRB_DoubleParam_IntFeasTol, 1e-2);

            // set Integrality Focus
            model.set(GRB_IntParam_IntegralityFocus, 1);

            // save the model
            model.write("EF1_LP_poolsearch_c++.lp");

            // set Objective: max (sum(lambda[i]*convexity[i])), maximize overall convexity
            // GRBLinExpr obj;
            // for(int i = 0; i < n; i++) {
            //     obj+=(Z[i]);
            // }
            // model.setObjective(obj, GRB_MINIMIZE);

            // optimize and print final values
            model.optimize();

            // check if model was feasible or not
            int optimstatus = model.get(GRB_IntAttr_Status);
            if (optimstatus == GRB_INF_OR_UNBD) {
                model.set(GRB_IntParam_Presolve, 0);
                model.optimize();
                optimstatus = model.get(GRB_IntAttr_Status);
                return 1;
            }
            if (optimstatus == GRB_OPTIMAL) {
                cout << "----------------------------------\n";
                cout << "Optimal Solution: \n";
                cout << "----------------------------------\n";

                // Print number of solutions stored
                int nSolutions = model.get(GRB_IntAttr_SolCount);
                cout << "Number of solutions found: " << nSolutions << endl;

                // Print all solutions
                for (int e = 0; e < nSolutions; e++) {

                    // access solution number `e`
                    model.set(GRB_IntParam_SolutionNumber, e);
                    // cout << model.get(GRB_DoubleAttr_PoolObjVal) << "\n";

                    cout << "Printing X ():" <<  e << "\n";
                    for(int i = 0; i < n; i++) {
                        for(int k = 0; k < m; k++) {
                            cout << round(abs(X[i][k].get(GRB_DoubleAttr_Xn))) << " ";
                        }
                        cout << "\n";
                    }
                    cout << "\n";

                    cout << "Printing Z: \n";
                    for(int i = 0; i < n; i++) {
                        cout << Z[i].get(GRB_DoubleAttr_Xn) << " ";
                    }
                    cout << "\n";

                    // check the solution is EF1 or not
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

        iteration++;
    }
    return 0;
}

