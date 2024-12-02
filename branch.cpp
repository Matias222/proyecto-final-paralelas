#include <bits/stdc++.h>
using namespace std;

// Constantes globales
int cota_ciudades = 100;      
int cota_poblacion = 200;     
int cota_iteraciones = 100000;  
double probabilidad_random = 0.15; 

vector<pair<int, int>> ciudades;

// Función para calcular la distancia entre dos ciudades
double calcularDistancia(int ciudad1, int ciudad2) {
    int x1 = ciudades[ciudad1].first, y1 = ciudades[ciudad1].second;
    int x2 = ciudades[ciudad2].first, y2 = ciudades[ciudad2].second;
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

// Función para calcular la longitud total del recorrido
double calcularDistanciaTotal(const vector<int>& tour) {
    double distanciaTotal = 0.0;
    for (size_t i = 0; i < tour.size() - 1; ++i) {
        distanciaTotal += calcularDistancia(tour[i], tour[i + 1]);
    }
    distanciaTotal += calcularDistancia(tour.back(), tour.front()); 
    return distanciaTotal;
}

// Heurística: calcular la suma de distancias mínimas desde el nodo actual
double heuristica(const vector<int>& recorrido, int nodoActual) {
    double costoHeuristico = 0.0;
    vector<bool> visitado(ciudades.size(), false);

    for (int nodo : recorrido) {
        visitado[nodo] = true;
    }

    for (size_t i = 0; i < ciudades.size(); ++i) {
        if (!visitado[i]) {
            double distanciaMinima = 1e9;
            for (size_t j = 0; j < ciudades.size(); ++j) {
                if (i != j && !visitado[j]) {
                    distanciaMinima = min(distanciaMinima, calcularDistancia(i, j));
                }
            }
            costoHeuristico += distanciaMinima;
        }
    }

    costoHeuristico += calcularDistancia(nodoActual, 0); // Distancia al nodo inicial
    return costoHeuristico;
}

// Lectura de datos de las ciudades
vector<pair<int,int>> lectura() {
    vector<pair<int,int>> temp;
    std::ifstream inputFile("data.txt");
    std::string line;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        int id, x, y;
        if (iss >> id >> x >> y) {
            temp.emplace_back(x, y);
        }
    }

    inputFile.close();
    return temp;
}

// Algoritmo Branch and Bound
void algo() {
    stack<vector<int>> pila;
    vector<int> mejorRecorrido;
    double mejorCosto = 1e9;

    // Inicializar el stack con el nodo inicial
    pila.push({0});

    while (!pila.empty()) {
        vector<int> recorridoActual = pila.top();
        pila.pop();

        double costoActual = calcularDistanciaTotal(recorridoActual);

        // Si la longitud del recorrido actual más la heurística supera el mejor costo, descartar
        if (costoActual + heuristica(recorridoActual, recorridoActual.back()) >= mejorCosto) {
            continue;
        }

        // Si es un recorrido completo
        if (recorridoActual.size() == ciudades.size()) {
            recorridoActual.push_back(0); // Regresar al inicio
            double costoTotal = calcularDistanciaTotal(recorridoActual);
            if (costoTotal < mejorCosto) {
                mejorCosto = costoTotal;
                mejorRecorrido = recorridoActual;
            }
            continue;
        }

        // Expandir los nodos hijos
        for (size_t i = 0; i < ciudades.size(); ++i) {
            if (find(recorridoActual.begin(), recorridoActual.end(), i) == recorridoActual.end()) {
                vector<int> nuevoRecorrido = recorridoActual;
                nuevoRecorrido.push_back(i);
                pila.push(nuevoRecorrido);
            }
        }
    }

    // Mostrar el mejor recorrido y su costo
    cout << "Mejor recorrido: ";
    for (int ciudad : mejorRecorrido) {
        cout << ciudad << " ";
    }
    cout << "\nCosto del mejor recorrido: " << mejorCosto << endl;
}

// Función principal
int main() {
    ciudades = lectura();
    algo();
    return 0;
}
