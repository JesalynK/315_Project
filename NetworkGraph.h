#ifndef NETWORK_GRAPH_H
#define NETWORK_GRAPH_H
#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include "Packet.h"
using namespace std;

inline string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cerr << "\nInput ended unexpectedly. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
    return line;
}
// keeps asking until the user enters a single whole number from minVal to maxVal to prevent loop of doom
// rejects letters, decimals, extra text, and out-of-range values like negs including negatives
inline int getValidInt(const string& prompt, int minVal, int maxVal) {
    while (true) {
        stringstream ss(readLine(prompt));
        long long value; // long long so huge numbers still get caught by the range check
        string leftover;

        if (!(ss >> value)) {
            cout << "  That's not a number. Please enter a whole number." << endl;
        } else if (ss >> leftover) {
            if (leftover[0] == '.') {
                cout << "  Decimals aren't allowed. Please enter a whole number." << endl;
            } else {
                cout << "  Please enter just one whole number (no extra text)." << endl;
            }
        } else if (value < minVal || value > maxVal) {
            cout << "  Out of range. Enter a number from " << minVal << " to " << maxVal << "." << endl;
        } else {
            return static_cast<int>(value); //already checked it fits in minVal, maxVal
        }
    }
}

// Asks a yes/no question. Accepts y, yes, n, no in any capitalization.
inline bool getYesNo(const string& prompt) {
    while (true) {
        stringstream ss(readLine(prompt));
        string answer;
        ss >> answer; // first word only, ignores extra spaces
        for (char& c : answer) {
            c = tolower((unsigned char)c);
        }

        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }
        cout << "  Please answer y or n." << endl;
    }
}

class Network {
    private:
        int V; //number of routers
        vector<vector<pair<int,int>>> adj; //adjacency list
    public:
        //Constuctor
        Network(int vertices) {
            V = vertices;
            adj.resize(V);
        }

        void addEdge(int u, int v, int weight); //adds an edge to adj
        int countVertices() const; //returns V so we know router amount;
        vector<int> getNextHop(int src); //getNextHop fcn using Dijkstras
        bool loadNetwork(const string& filename); //James: file parser, reads a topology file into this graph
        static Network loadingNetwork(const string& filename); //asks the user for a network, saves it to a file, then loads that file
        bool hasEdge(int u, int v) const; //true if routers u and v are already linked
        int getCount(int source, int destination); // returns the weight between 2 networks
};

void Network::addEdge(int u, int v, int weight) {
    adj[u].push_back({v, weight});
    adj[v].push_back({u, weight}); //we are assuming undirected network lines here
}

int Network::countVertices() const {
    return V;
}

vector<int> Network::getNextHop(int src) {
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq; //establishes priority queue as a min heap
    vector<int> dist(V, INT_MAX);
    vector<int> parent(V, -1);

    dist[src] = 0; //distance from itself is 0
    pq.emplace(0,src); //push the source into the pq

    while(!pq.empty()) {
        auto top = pq.top();
        pq.pop();
        int d = top.first;
        int u = top.second;

        if(d > dist[u]) continue;
        for (auto &p : adj[u]) { //loop through adjacent edges connected to the current vertex u
            int v = p.first; //neighboring vertex
            int w = p.second; //the weight (COST)

            // If we found a shorter path to v through u, update it
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w; 
                parent[v] = u; //record preceding node  
                pq.emplace(dist[v], v); //update
            }
        }
    }

    vector<int> nextHop(V,-1);
    for(int i = 0; i < V; ++i) { //looping through every destination
        if(i == src) continue; //skips source

        int curr = i;
        while(src != parent[curr] && parent[curr] != -1) { //look at the parents until the node we are looking at was reached from our source
            curr = parent[curr];
        }
        if(parent[curr] == src) {
            nextHop[i] = curr; //store the next hop destination (curr is looking at what node needs to be hit next to reach dest i)
        }
    }
    // Return the next hop to get to location
    return nextHop;
}

// skips blank lines and # comments. Bad edge lines are reported with their line number and skipped so user knows what they did wrong
// returns false if the file can't be opened or has no valid router count so  no errors are thrown, just a false return to indicate failure
bool Network::loadNetwork(const string& filename) {
    ifstream in(filename);
    if (!in) {
        cerr << "Error: could not open " << filename << endl;
        return false;
    }

    string line;
    int lineNum = 0;
    bool gotCount = false;

    while (getline(in, line)) {
        lineNum++;
        if (!line.empty() && line.back() == '\r') { // windows line endings
            line.pop_back();
        }

        size_t first = line.find_first_not_of(" \t");
        if (first == string::npos || line[first] == '#') { // blank, whitespace-only, or comment
            continue;
        }

        stringstream ss(line);

        if (!gotCount) { // first data line is the router count
            int count;
            if (!(ss >> count) || count < 1 || count > 10) {
                cerr << "Error (line " << lineNum << "): router count must be 1-10, got \"" << line << "\"" << endl;
                return false;
            }
            V = count;
            adj.assign(V, {});
            gotCount = true;
            continue;
        }

        // for the rest of the lines, expect "u v w" for an edge
        int u, v, w;
        if (!(ss >> u >> v >> w)) {
            cerr << "Warning (line " << lineNum << "): expected 3 numbers, skipping \"" << line << "\"" << endl; // bad line didnt match the expected format SKIP it
        } else if (u < 0 || u >= V || v < 0 || v >= V) {
            cerr << "Warning (line " << lineNum << "): router must be 0-" << V - 1 << ", skipping" << endl; // bad line router out of bounds SKIP it
        } else if (u == v) {
            cerr << "Warning (line " << lineNum << "): router can't connect to itself, skipping" << endl; // bad line, used the same router for both ends, SKIP
        } else if (w < 1 || w > 10000) {
            cerr << "Warning (line " << lineNum << "): weight must be 1-10000, skipping" << endl; // bad line, weight out of bounds, SKIP
        } else if (hasEdge(u, v)) {
            cerr << "Warning (line " << lineNum << "): routers " << u << " and " << v << " already connected, skipping" << endl; // bad line, already connected, SKIP
        } else {
            addEdge(u, v, w);
        }
    }

    if (!gotCount) {
        cerr << "Error: " << filename << " has no router count" << endl; //for empty files or only comments/blanks
        return false;
    }
    return true;
}

// true if routers u and v already have a link between them
bool Network::hasEdge(int u, int v) const {
    for (const auto& edge : adj[u]) {
        if (edge.first == v) {
            return true;
        }
    }
    return false;
}

// ask user to build a network, saves it to filename, then loads it back with the parser.
// static, so it's called as Network::loadingNetwork without needing an existing Network.
Network Network::loadingNetwork(const string& filename) {
    int routers = getValidInt("Enter the number of routers you want (1-10): ", 1, 10);

    // linked a b is true once a and b are connected, so 0-1 and 1-0 can't both be added
    vector<vector<bool>> linked(routers, vector<bool>(routers, false));
    vector<string> edgeLines; // "u v w" lines for the file

    for (int i = 0; i < routers && routers > 1; ++i) { // a single router has nothing to connect to
        cout << endl << "Router " << i << ":" << endl;

        while (getYesNo("  Add a connection from router " + to_string(i) + "? (y/n): ")) {
            int dest = getValidInt("    Connected to router (0-" + to_string(routers - 1) + "): ", 0, routers - 1);

            if (dest == i) {
                cout << "    A router can't connect to itself." << endl;
                continue;
            }
            if (linked[i][dest]) {
                cout << "    Router " << i << " and router " << dest << " are already connected." << endl;
                continue;
            }

            int weight = getValidInt("    Latency of this link in ms (1-10000): ", 1, 10000);

            linked[i][dest] = true;
            linked[dest][i] = true; // links go both ways
            edgeLines.push_back(to_string(i) + " " + to_string(dest) + " " + to_string(weight));
        }
    }

    ofstream file(filename);
    if (!file) {
        cerr << "Error: could not write " << filename << endl;
        exit(EXIT_FAILURE);
    }
    file << "# Number of Routers" << endl;
    file << routers << endl;
    file << "# Edges: Source, Destination, Weight (Latency in ms)" << endl;
    for (const string& edge : edgeLines) {
        file << edge << endl;
    }
    file.close(); // has to be closed before we read it back

    Network net(0);
    if (!net.loadNetwork(filename)) {
        exit(EXIT_FAILURE);
    }
    cout << endl << "Loaded " << net.countVertices() << " routers and " << edgeLines.size()
         << " connections from " << filename << endl;
    return net;
}

int Network::getCount(int source, int destination) {
    for (const auto& p : adj[source]) {
        if (p.first == destination) {
            return p.second;
        }
    }

    return -100;
}

#endif
