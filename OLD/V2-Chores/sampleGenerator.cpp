#include <bits/stdc++.h>
using namespace std;

int main() {
    ofstream myfile;
    for(int i = 0; i < 1000; i++) {
        srand(i);
        string fileName = "utilities" + to_string(i) + ".txt";
        myfile.open (fileName);
        int n = rand() % 10 + 2; 
        int m = rand() % 15 + 1;
        myfile << n << " " << m << endl;
        for(int j = 0; j < m; j++) {
            for(int i = 0; i < n; i++) {
                int vij = (rand() % 15 + 1)*10000000;
                myfile << vij << " ";
            }
            myfile << endl;
        }
        myfile.close();
    }

    cout << "Sample Generation Complete." << endl;
    
    return 0;
}