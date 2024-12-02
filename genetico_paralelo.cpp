#include <bits/stdc++.h>
#include "omp.h"

using namespace std::chrono;
using namespace std;

const int cota_ciudades = 76;      
const int cota_poblacion = 200;     
const int cota_iteraciones = 500000;  
const int mejor_tour = 108159;  //108159 //21282
 
const double probabilidad_random = 0.15; 

const string archivo="data_76.txt";

vector<pair<double, double>> ciudades;
vector<int> curva_optima;

auto start = high_resolution_clock::now();


void writeVectorToFile() {
    std::ofstream outFile("genetico_paralelo_funcion.txt");

    for (const int& num : curva_optima) {
        outFile << num << std::endl;
    }

    outFile.close();
}

double calcularDistancia(int ciudad1, int ciudad2) {
    double x1 = ciudades[ciudad1].first, y1 = ciudades[ciudad1].second;
    double x2 = ciudades[ciudad2].first, y2 = ciudades[ciudad2].second;
    return round(sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)));
}

double calcularDistanciaTotal(const vector<int>& tour) {
    double distanciaTotal = 0.0;
    for (size_t i = 0; i < tour.size() - 1; ++i) {
        distanciaTotal += calcularDistancia(tour[i], tour[i + 1]);
    }
    distanciaTotal += calcularDistancia(tour.back(), tour.front()); 
    return distanciaTotal;
}

vector<pair<double,double>> lectura(){
    
    vector<pair<double,double>> temp;

    std::ifstream inputFile(archivo); // Replace "input.txt" with the path to your file

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        double id, x, y;
        if (iss >> id >> x >> y) {
            temp.emplace_back(x, y); // Store only x and y as a pair
        }
    }

    inputFile.close();

    // Output the parsed vector for verification
    for (const auto& point : temp) {
        std::cout << "(" << point.first << ", " << point.second << ")" << std::endl;
    }
    return temp;
}

vector<vector<int>> generarPoblacionAleatoria(int numCiudades, int tamanoPoblacion) {
    
    vector<int> baseTour(numCiudades);
    iota(baseTour.begin(), baseTour.end(), 0);

    vector<vector<int>> poblacion;
    random_device rd;
    mt19937 generador(123);

    for (int i = 0; i < tamanoPoblacion; ++i) {
        shuffle(baseTour.begin(), baseTour.end(), generador);
        poblacion.push_back(baseTour);
    }

    return poblacion;
}

int elegirAleatorio(int cota, int repetido) {
    while (true) {
        int diferente = rand() % cota;
        if (diferente != repetido) return diferente;
    }
}

void genetico(vector<vector<int>>& poblacion) {

    double anteriorbest=-1;
    int estancado=0;

    for (int iteracion = 0; iteracion < cota_iteraciones; ++iteracion) {

        vector<vector<int>> temporal(cota_poblacion);

        #pragma omp parallel for
        for (int j = 0; j < cota_poblacion; ++j) {

            vector<int> nuevoTour = poblacion[j];
            bool repetir = true;

            int ciudadInicio = nuevoTour[rand() % cota_ciudades];
            int ciudadFinal;

            while (repetir) {
                
                if ((rand() % 20) < (probabilidad_random * 20)) {
                    ciudadFinal = elegirAleatorio(cota_ciudades, ciudadInicio);
                } else {
                    
                    while(1){
                    
                        int otroTourIndice = elegirAleatorio(cota_poblacion, j);
                        auto it = find(poblacion[otroTourIndice].begin(), poblacion[otroTourIndice].end(), ciudadInicio);
                        if (it != poblacion[otroTourIndice].end() && next(it) != poblacion[otroTourIndice].end()) {
                            ciudadFinal = *next(it);
                            break;                       
                        }
                    }

                }

                if (ciudadInicio+1==cota_ciudades || nuevoTour[ciudadInicio+1]==ciudadFinal) {
                    repetir = false;
                } else {

                    auto it1 = find(nuevoTour.begin(), nuevoTour.end(), ciudadInicio);
                    auto it2 = find(nuevoTour.begin(), nuevoTour.end(), ciudadFinal);
                    // Reverse section between ciudadInicio and ciudadFinal
                    if (it1 > it2) swap(it1, it2);
                    reverse(it1 + 1, it2 + 1);
                    ciudadInicio = ciudadFinal;
                }
            }

            if ( calcularDistanciaTotal(nuevoTour) < calcularDistanciaTotal(poblacion[j])) temporal[j]=nuevoTour;
            else temporal[j]=poblacion[j];
            
        }

        poblacion=temporal;

        double mejorDistancia = numeric_limits<double>::max();
        vector<int> mejorTour;

        for (const auto& tour : poblacion) {
            double distancia = calcularDistanciaTotal(tour);
            if (distancia < mejorDistancia) {
                mejorDistancia = distancia;
                mejorTour = tour;
            }
        }

        curva_optima.push_back(int(mejorDistancia));

        if(iteracion%100==0) cout << "Iteracion " << iteracion + 1 << ": Mejor Distancia = " << mejorDistancia << " Estanco = "<<estancado<<endl;

        if(mejorDistancia==mejor_tour){
            auto stop = high_resolution_clock::now(); // Detén el cronómetro
            auto duration = duration_cast<seconds>(stop - start); // Calcula la duración
            cout << "Tiempo en segundos: " << duration.count() << "s" << endl;
            return;
        }

        if(mejorDistancia==anteriorbest){
            estancado++;
        }else{
            anteriorbest=mejorDistancia;
            estancado=0;
        }
    
    }
}

int main() {

    ciudades=lectura();

    srand(static_cast<unsigned int>(36)); 

    auto poblacion = generarPoblacionAleatoria(cota_ciudades, cota_poblacion);

    genetico(poblacion);
    
    writeVectorToFile();

    return 0;
}
