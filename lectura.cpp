#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>

// Structure to hold node information
struct Node {
    int id;
    double x;
    double y;
};

// Function to calculate Euclidean distance between two nodes
double euclideanDistance(const Node& a, const Node& b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

int main() {

    std::ifstream file("xqf131.tsp");

    std::string line;
    std::vector<Node> nodes;
    bool inNodeCoordSection = false;

    // Read the file line by line
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string word;
        iss >> word;

        if (word == "NODE_COORD_SECTION") {
            inNodeCoordSection = true;
            continue;
        }

        if (word == "EOF") {
            break;
        }

        if (inNodeCoordSection) {
            Node node;
            iss >> node.id >> node.x >> node.y;
            nodes.push_back(node);
        }
    }

    file.close();

    int dimension = nodes.size();
    std::vector<std::vector<double>> distanceMatrix(dimension, std::vector<double>(dimension, 0.0));

    // Compute the distance matrix
    for (int i = 0; i < dimension; ++i) {
        for (int j = 0; j < dimension; ++j) {
            if (i != j) {
                distanceMatrix[i][j] = euclideanDistance(nodes[i], nodes[j]);
            }
        }
    }

    // Output the distance matrix
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& row : distanceMatrix) {
        for (double dist : row) {
            std::cout << dist << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
