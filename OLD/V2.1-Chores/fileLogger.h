#include <bits/stdc++.h>

using namespace std;

class fileHandler {
    public:
    ofstream myExcel;
    ofstream logfile;                          // logs the entire output
    ofstream repeatFile;
    ofstream sampleFile;                       // logs the sample
    ofstream minEnvyDiff_File;                 // tracks the minimum of [d_i(X_h) - d_i(X_i) + max(d_ij)] over all i.j 
    ofstream minBundlePrice_File;              // tracks the minimum bundle price over iterations
    ofstream EFMaxValuation_File;              // tracks di(Xi) - di_max = disutility of least spender after removing highest disutility chore
    ofstream EFMaxBundlePrice_File;            // tracks EFMax bundle price over iterations
    ofstream nashEFMaxWelfare_File;            // tracks product of EFMax Valuations of all agents ~ Nash Welfare (if > 0)
    ofstream minBundleValuation_File;          // tracks the disutility/valuation of the minimum bundle
    ofstream EFMaxPlusMinValuation_File;       // tracks d_i(Xi) - d_i_max + d_i_min of least spender 
    ofstream minAndEFMaxBundlePriceDiff_File;  // tracks the difference between the minimum bundle price and the EFMax bundle Price
    ofstream LSValAndBSEFMaxValDiff_wrtBS_File; // tracks d_BS(X_LS) - d_BS(X_BS\g) after every transfer path completes

    vector<double> minBundlePrice_vec;
    vector<double> EFMAxBundlePrice_vec;
    

    fileHandler() {
        myExcel.open("./Logs/ExcelLog.txt", std::ios_base::app);
        logfile.open("./Logs/Log.txt");
        repeatFile.open("./Logs/repeatLog.txt");
        sampleFile.open("./Logs/Samples.txt");
        minEnvyDiff_File.open("./Logs/MinEnvyDiff.txt");
        minBundlePrice_File.open("./Logs/MinBundlePrice.txt");
        EFMaxValuation_File.open("./Logs/EFMaxValuation.txt");
        EFMaxBundlePrice_File.open("./Logs/EFMaxBundlePrice.txt");
        nashEFMaxWelfare_File.open("./Logs/nashEFMaxWelfare_File.txt");
        minBundleValuation_File.open("./Logs/MinBundleValuation.txt");
        EFMaxPlusMinValuation_File.open("./Logs/EFMaxPlusMinValuation_File.txt");
        minAndEFMaxBundlePriceDiff_File.open("./Logs/MinAndEFMaxBundlePriceDiff.txt");
        LSValAndBSEFMaxValDiff_wrtBS_File.open("./Logs/LSValAndBSEFMaxValDiff_wrtBS.txt");
        
        // defining initial headers in a file
        logfile << "Iteration" << " " << "Agents" << " " << "Items" << " " << "Duration" << " " << "Price_Rise_Steps" << " " << "Transfer_Steps" << endl;
        minEnvyDiff_File.precision(1);
        EFMaxValuation_File.precision(1);
        nashEFMaxWelfare_File.precision(1);
        EFMaxPlusMinValuation_File.precision(1);
        minAndEFMaxBundlePriceDiff_File.precision(1);
    }

    void logIteration(int iteration, int n, int m) {
        myExcel << endl << "Sample-" <<  iteration << endl;
        logfile << iteration << " " << n << " " << m << " ";
        repeatFile << iteration << " ";
        minEnvyDiff_File << iteration << " ";
        minBundlePrice_File << iteration << " ";
        EFMaxValuation_File << iteration << " ";
        EFMaxBundlePrice_File << iteration << " ";
        nashEFMaxWelfare_File << iteration << " ";
        minBundleValuation_File << iteration << " ";
        EFMaxPlusMinValuation_File << iteration << " ";
    }

    void nextIteration() {
        myExcel << endl;
        logfile << endl;
        repeatFile << endl;
        minEnvyDiff_File << endl;
        minBundlePrice_File << endl;
        EFMaxValuation_File << endl;
        EFMaxBundlePrice_File << endl;
        minBundleValuation_File << endl;
        minAndEFMaxBundlePriceDiff_File << endl;

        minBundlePrice_vec.clear();
        EFMAxBundlePrice_vec.clear();
    }

    void closeFiles() {
        myExcel.close();
        logfile.close();
        repeatFile.close();
        sampleFile.close();
        minEnvyDiff_File.close();
        minBundlePrice_File.close();
        EFMaxValuation_File.close();
        EFMaxBundlePrice_File.close();
        minBundleValuation_File.close();
        minAndEFMaxBundlePriceDiff_File.close();
    }

    void logValIntoFile(ofstream &myFile, double val, int val2=-1) {
        myFile << val << " " << ((val2!=-1)?(to_string(val2)+" ; "):"");
    }
};