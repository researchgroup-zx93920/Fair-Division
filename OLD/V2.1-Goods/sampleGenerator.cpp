#include <bits/stdc++.h>
#include <stdlib.h>
#include <pcg_random.hpp>
using namespace std;

int main() {
    ofstream myfile;
    std::uniform_int_distribution<int> uniform_dist(1, 6);
    for(int i = 0; i < 12; i++) {

        // Seed with a real random value, if available
        // pcg_extras::seed_seq_from<std::random_device> seed_source;

        // Make a random number engine
        pcg32 rng(i);

        // Choose a random mean between 1 and 6
        int mean = uniform_dist(rng);
        cout << "RN is " << mean << endl;
        mean = uniform_dist(rng);
        cout << "RN is " << mean << endl;
        mean = uniform_dist(rng);
        cout << "RN is " << mean << endl;

        srand(i);
        string fileName = "utilities" + to_string(i) + ".txt";
        myfile.open (fileName);
        int n = (int) rand() % 10 + 2; 
        int m = (int) rand() % 15 + 1;
        myfile << n << " " << m << endl;
        for(int j = 0; j < m; j++) {
            for(int i = 0; i < n; i++) {
                int vij = rand() % 15 + 1;
                myfile << vij << " ";
            }
            myfile << endl;
        }
        myfile.close();
    }

    cout << "Sample Generation Complete." << endl;
    
    return 0;
}