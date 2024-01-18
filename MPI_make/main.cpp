#include <iostream>
#include "MPI_dijkstra.h"

using namespace std;

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        cerr << "Please args -n, exe, vertex_size, src_vertex, input_file!\n";
        return -1;
    }

    MPI_dijkstra md(stoi(argv[argc - 3]), stoi(argv[argc - 2]), argv[argc - 1]);
    md.mpi_dijkstra_calc(argc - 3, argv);

    return 0;
}