// This is CS315 project which implements Dijkstras akgorithm to find the shortest path and simulates packets routing from router to router.
// the Dijkstras algorithm was found online to help implementation. Using this link:https://www.geeksforgeeks.org/dsa/dijkstras-shortest-path-algorithm-greedy-algo-7/
#include <iostream>
#include <fstream>
#include <vector>
#include "Packet.h"
#include "NetworkGraph.h"
using namespace std;

void drawGraph(Network network, int source, int destination) {
    // create a text file to store the ascii art
    ofstream graphFile("Graph.txt", ios::app);
    if (!graphFile.is_open()) {
        cerr << "Error in openning file!" << endl;
        return;
    }
    
    // create each node with each having a height of the number of nodes
    graphFile << " _______ " << endl;
    graphFile << "|       |" << endl;

    int spaces = 3 - (source >= 10 ? 1 : 0); 
    graphFile << "|   " <<  source << string(spaces, ' ') << "|" << endl;
    
    graphFile << "|_______|" << endl;
    
    // getting the weight between the 2 nodes
    int weight = network.getCount(source, destination);
    
    if (weight != -100) {
        graphFile << "    |" << endl;
        graphFile << "    | " <<  weight << endl;
        graphFile << "    |" << endl;
        graphFile << "    v" << endl;
    }
    
    graphFile.close();
}

// returns true if succesful and false if failed
bool simulatePackets(Network network, Packet package) {
    int cost = 0;
    vector<int> steps = {package.source};
    int ttlCount = package.ttl;
    
    // check if destination is source, if so success
    while (package.dest != steps.back()) {
        // check to make sure ttl isnt gone
        if (ttlCount-- <= 0) {
            cerr << "Package time to live has expired." << endl;
            return false; // failed delivery
        }
        
        // give out the best route to destination using the next hop fcn
        vector<int> hopOptions = network.getNextHop(steps.back());
        steps.push_back(hopOptions[package.dest]);
        
        // give out the cost of this route
        cost += network.getCount(steps[steps.size() - 2], steps.back());

        // if next hop is -1 then the packet was dropped
        if (steps.back() == -1) {
            cout << "Package was dropped." << endl;
            break;
        }
    }
    
    // resetting text file
    ofstream graphFile("Graph.txt");
    if (!graphFile.is_open()) {
        cerr << "Error in opening file!" << endl;
        return false;
    }
    graphFile << "Fastest route to destination:" << endl;
    graphFile.close();
    
    // printing best route
    for (int i = 0; i < steps.size(); i++) {
        if (i + 1 == steps.size()) {
            drawGraph(network, steps[i], steps[i]);
        }
        else {
            drawGraph(network, steps[i], steps[i + 1]);
        }
    }
    cout << endl;
    
    // also give out the cost of this route
    cout << "Total cost of delivery: " << cost << endl;
    
    // outputing time in system
    cout << "Time in system: " << package.ttl - ttlCount << endl;
    cout << "Time left in package: " << ttlCount << endl;

    return true; // succesful delivery
}

// asks the user for the packet's route, TTL, and message
Packet getPackage(const Network& net) {
    int lastRouter = net.countVertices() - 1;

    cout << endl << "Time to get a packet!" << endl;
    int source = getValidInt("What router is the packet starting at? (0-" + to_string(lastRouter) + "): ", 0, lastRouter);
    int dest = getValidInt("What router is the packet going to? (0-" + to_string(lastRouter) + "): ", 0, lastRouter);
    int ttl = getValidInt("What is the packet's time to live? (1-255): ", 1, 255);
    string message = readLine("Message to send: ");

    return Packet(1, source, dest, ttl, message);
}

int main() {
    //main right now is a bunch of tests to make sure my algorithm works properly
    Network network = Network::loadingNetwork("network.txt"); //asks user for network, saves it, loads it back
    Packet package = getPackage(network);
    simulatePackets(network, package);
    return 0;
}
