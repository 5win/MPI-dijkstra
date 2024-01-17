#include <iostream>
#include "MPI_dijkstra.h"

using namespace std;

int main(int argc, char* argv[]) {

    MPI_dijkstra md;
    md.N = stoi(argv[argc - 2]);
    md.startVertex = stoi(argv[argc - 1]);
    
    md.mpi_dijkstra_calc(argc - 2, argv);


    cout << "main exit!\n";

    return 0;
}