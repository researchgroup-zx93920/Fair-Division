// C++ Program to implement
// the above approach
#include <bits/stdc++.h>
using namespace std;

// Function to display adjacency list
void printList(vector<vector<int>> adj, int V)
{
	for (int i = 0; i < V; i++) {
		cout << i;
		for (auto j : adj[i])
			cout << " --> " << j;
		cout << endl;
	}
	cout << endl;
}

// Function to convert adjacency
// list to adjacency matrix
vector<vector<int> > convert(vector<vector<int>> adj, int V)
{
	// Initialize a matrix
	vector<vector<int>> matrix(V,vector<int>(V, 0));

	for (int i = 0; i < V; i++) {
		for (auto j : adj[i])
			matrix[i][j] = 1;
	}
	return matrix;
}

// Function to display adjacency matrix
void printMatrix(vector<vector<int>> adj, int V)
{
	for (int i = 0; i < V; i++) {
		for (int j = 0; j < V; j++) {
			cout << adj[i][j] << ", ";
		}
		cout << endl;
	}
	cout << endl;
}

// Driver code
int main()
{
	

	vector<vector<int>> adjList = {{126, 127, 134}, {101, 115, 135, 139, 147, 154}, {145, 158, 161, 162}, {126, 141}, {106, 111, 130, 142}, {108, 110, 123, 142, 160}, {105, 153}, {114, 136}, {133, 135, 143, 148}, {125, 129, 144}, {114, 123}, {121}, {117, 150, 158}, {103, 110, 115, 144}, {109, 117, 128, 152, 153, 155, 159}, {119, 121, 132, 137, 145, 156}, {150}, {107, 131, 146, 149, 162}, {159}, {116, 119}, {157}, {160}, {102, 112, 113, 127, 157}, {125, 140}, {124, 151}, {133}, {104, 120, 132, 138}, {128}, {137}, {152}, {103}, {108, 122, 149}, {100, 116}, {118, 124, 131}, {106, 112}, {130, 154}};
  
	int V = adjList.size();
    // Display adjacency list
	cout << "Adjacency List: \n";
	printList(adjList, V);

	// Function call which returns
	// adjacency matrix after conversion
	vector<vector<int> > adjMatrix = convert(adjList, V);

	// Display adjacency matrix
	cout << "Adjacency Matrix: \n";
	printMatrix(adjMatrix, V);

	return 0;
}