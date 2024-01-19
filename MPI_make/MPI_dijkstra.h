#include <vector>
#include <mpi.h>
#include <string>

using namespace std;

// #define CV_INT int
// #define CW_INT int
typedef int CV_INT;
typedef int CW_INT;

#define CW_INT_MAX INT32_MAX
#define CMPI_DTYPE MPI_INT
// #define ROOT 0

struct Node {
    CV_INT dst;
    CW_INT weight;
};

class MPI_dijkstra {
    private:
        CV_INT N, startVertex, ROOT;
        string input_file;

    public:
        MPI_dijkstra(CV_INT n, CV_INT src, string f) : N(n), startVertex(src), input_file(f) {
            ROOT = 0;
        }
        void mpi_dijkstra_calc(int argc, char *argv[]);
    private:
        static void myMax(void* in, void* inout, int* len, MPI_Datatype* dptr);
        void read_graph_data(vector<vector<Node>>& graph, int& rank, int& comm_size, std::pair<CV_INT, CV_INT>& range);
        void relax(vector<vector<Node>>& graph, vector<CW_INT>& tent, vector<bool>& visited, Node* latest_visited, pair<CV_INT, CV_INT>& range);
};