#include <bits/stdc++.h>
#include "omp.h"
#include <mpi.h>

using namespace std;

int world_size, world_rank, veces_estancado=1;    
int bloqueo=1;
double start_time;

const double probabilidad_random = 0.15; 

const int cota_ciudades = 29;      
const int cota_poblacion = 200; 
const int mejor_tour = 1000 ; //108159 //21282 //131->564
const int cota_iteraciones = 250000;  

const string archivo="data_corta.txt";

vector<pair<double, double>> ciudades;
vector<int> curva_optima;

double poblaciones[cota_poblacion][cota_ciudades];


void writeVectorToFile() {
    std::ofstream outFile("genetico_paralelo_hibrido_funcion.txt");

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

    ifstream inputFile(archivo);

    string line;
    while (getline(inputFile, line)) {
        istringstream iss(line);
        double id, x, y;
        if (iss >> id >> x >> y) {
            temp.emplace_back(x, y);
        }
    }

    inputFile.close();

    return temp;
}

void generarPoblacionAleatoria(int numCiudades, int tamanoPoblacion) {
    
    vector<int> baseTour(numCiudades);
    iota(baseTour.begin(), baseTour.end(), 0);

    random_device rd;
    mt19937 generador(123);

    for (int i = 0; i < tamanoPoblacion; ++i) {
        shuffle(baseTour.begin(), baseTour.end(), generador);
        for (int j = 0; j < numCiudades; ++j) {
            poblaciones[i][j] = baseTour[j];
        }
    }
}

int elegirAleatorio(int cota, int repetido) {
    while (true) {
        int diferente = rand() % cota;
        if (diferente != repetido) return diferente;
    }
}

void genetico(int cota_iteraciones_recursivo) {


    struct { int value; int rank; } local, global;

    while(1){

        if(bloqueo==0 || world_rank==0 ){

            double anteriorbest = -1;
            int estancado = 0;
            double mejorDistanciaGlobal = numeric_limits<double>::max();

            for (int iteracion = 0; iteracion < cota_iteraciones_recursivo; ++iteracion) {

                int temporal[cota_poblacion][cota_ciudades];

                #pragma omp parallel for
                for (int j = 0; j < cota_poblacion; ++j) {

                    vector<int> nuevoTour(cota_ciudades);

                    for (int k = 0; k < cota_ciudades; ++k) {
                        nuevoTour[k] = poblaciones[j][k];
                    }

                    bool repetir = true;

                    int ciudadInicio = nuevoTour[rand() % cota_ciudades];
                    int ciudadFinal;

                    while (repetir) {
                        
                        if ((rand() % 20) < (probabilidad_random * 20)) {
                            ciudadFinal = elegirAleatorio(cota_ciudades, ciudadInicio);
                        } else {
                            
                            while(1){
                            
                                int otroTourIndice = elegirAleatorio(cota_poblacion, j);
                                auto it = find(poblaciones[otroTourIndice], poblaciones[otroTourIndice] + cota_ciudades, ciudadInicio);
                                if (it != poblaciones[otroTourIndice] + cota_ciudades && (it + 1) != poblaciones[otroTourIndice] + cota_ciudades) {
                                    ciudadFinal = *(it + 1);
                                    break;                       
                                }
                            }

                        }

                        if (ciudadInicio + 1 == cota_ciudades || nuevoTour[ciudadInicio + 1] == ciudadFinal) {
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

                    if (calcularDistanciaTotal(nuevoTour) < calcularDistanciaTotal(vector<int>(poblaciones[j], poblaciones[j] + cota_ciudades))) {
                        copy(nuevoTour.begin(), nuevoTour.end(), temporal[j]);
                    } else {
                        copy(poblaciones[j], poblaciones[j] + cota_ciudades, temporal[j]);
                    }
                }

                for (int j = 0; j < cota_poblacion; ++j) copy(temporal[j], temporal[j] + cota_ciudades, poblaciones[j]);
                
                double mejorDistancia = numeric_limits<double>::max();
                vector<int> mejorTour;

                for (int i = 0; i < cota_poblacion; ++i) {
                    vector<int> tour(poblaciones[i], poblaciones[i] + cota_ciudades);
                    double distancia = calcularDistanciaTotal(tour);
                    if (distancia < mejorDistancia) {
                        mejorDistancia = distancia;
                        mejorTour = tour;
                    }
                }

                mejorDistanciaGlobal=min(mejorDistanciaGlobal,mejorDistancia);

                if(iteracion%100==0) cout << "Iteracion " << iteracion + 1 << ": Mejor Distancia = " << mejorDistancia << " Estanco = " << estancado << endl;

                if(world_rank==0){

                    curva_optima.push_back(mejorDistanciaGlobal);

                    if ( (estancado+1) % 850 == 0 ) {

                        cout << "Estancado" << '\n';
                        //Broadcast del checkpoint de donde nos ubicamos y paralelizacion?

                        int secundarios_iteraciones = 800*veces_estancado;

                        veces_estancado=veces_estancado*2;

                        bloqueo=0;
                        MPI_Bcast(&bloqueo, 1 , MPI_INT, 0, MPI_COMM_WORLD);
                        MPI_Bcast(&secundarios_iteraciones, 1, MPI_INT, 0, MPI_COMM_WORLD);
                        MPI_Bcast(poblaciones, cota_ciudades * cota_poblacion, MPI_DOUBLE, 0, MPI_COMM_WORLD);

                        //espera al brodcast de los otros procesos 

                        local.rank=0;
                        local.value=1e9;

                        MPI_Barrier(MPI_COMM_WORLD);

                        MPI_Allreduce(&local, &global, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
                        
                        cout<<"MINIMO GLOBAL DESDE MAIN "<<global.value<<" "<<global.rank<<"\n";

                        MPI_Bcast(poblaciones, cota_ciudades * cota_poblacion, MPI_DOUBLE, global.rank, MPI_COMM_WORLD);
                        
                        cout<<"Me regresaron el poder"<<'\n';
                    }   

                    if(mejorDistanciaGlobal<=mejor_tour){

                        double end_time = MPI_Wtime();
                        double local_elapsed = end_time - start_time;
                        
                        cout << "Tiempo a la respuesta: "<< local_elapsed << " seconds" << std::endl;

                        return;
                    }


                    if (mejorDistancia == anteriorbest) {
                        estancado++;
                    } else {
                        anteriorbest = mejorDistancia;
                        estancado = 0;
                        veces_estancado=1;
                    }
                }
            }
            
            if(world_rank!=0){ //Conservar el mejor de los n procesors

                bloqueo=1;

                local.value = int(mejorDistanciaGlobal);
                local.rank = world_rank;

                MPI_Barrier(MPI_COMM_WORLD);

                MPI_Allreduce(&local, &global, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

                cout<<"MINIMO GLOBAL DESDE OTROS "<<global.value<<" "<<global.rank<<"\n";

                MPI_Bcast(poblaciones, cota_ciudades * cota_poblacion, MPI_DOUBLE, global.rank, MPI_COMM_WORLD);

            }else break;
        
        }
        
        cout<<"Proceso "<<world_rank<<" esperando broadcast"<<"\n";

        MPI_Bcast(&bloqueo, 1 , MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&cota_iteraciones_recursivo, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(poblaciones, cota_ciudades * cota_poblacion, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        cout<<"Todo recibido "<<bloqueo<<' '<<cota_iteraciones_recursivo<<"\n";

    }

}

int main() {

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    ciudades = lectura();

    double start_time = MPI_Wtime();

    if (world_rank == 0) {
        srand(static_cast<unsigned int>(36)); 
        generarPoblacionAleatoria(cota_ciudades, cota_poblacion);
        MPI_Bcast(poblaciones, cota_ciudades*cota_poblacion, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }else{
        srand(static_cast<unsigned int>(std::time(0)+world_rank)); 
        MPI_Bcast(poblaciones, cota_ciudades*cota_poblacion, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    genetico(cota_iteraciones);

    writeVectorToFile();

    MPI_Finalize();

    return 0;
}
