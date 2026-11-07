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

    // read text file and display node
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
    for (int i = 0; i+1 < steps.size(); i++) {
        drawGraph(network, steps[i], steps[i + 1]);
    }
    cout << endl;
    
    // also give out the cost of this route
    cout << "Total cost of delivery: " << cost << endl;
    
    // outputing time in system
    cout << "Time in system: " << package.ttl - ttlCount << endl;
    cout << "Time left in package: " << ttlCount << endl;

    return true; // succesful delivery
}

Packet getPackage(Network net) {
    Packet packet(0,-1,-1,-1,"");
    cout<<endl;
    cout<<"Time to get a packet! What router is the packet starting at? ";
    cin>>packet.source;
    while(packet.source < 0 || packet.source > net.countVertices()) {
        cout<<"Error! source does not exist! Try again: ";
        cin>>packet.source;
    }
    cout<<"What router is the packet going to? ";
    cin>>packet.dest;
    while(packet.dest < 0 || packet.dest > net.countVertices()) {
        cout<<"Error! Destination does not exist! Try again: ";
        cin>>packet.dest;
    }
    cout<<"What is the packets time to live? ";
    cin>>packet.ttl;
    while(packet.ttl <= 0) {
        cout<<"Error! Time to live must be greater than 0! Try again: ";
        cin>>packet.ttl;
    }
    return packet;
}

int main() {
    //main right now is a bunch of tests to make sure my algorithm works properly
    Network network = network.loadingNetwork("network.txt"); //asks user for network
    Packet package = getPackage(network);
    simulatePackets(network, package);
    return 0;
}