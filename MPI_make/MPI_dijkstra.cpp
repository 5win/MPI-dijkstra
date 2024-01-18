#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#include "MPI_dijkstra.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

using namespace std;

void MPI_dijkstra::myMax(void* in, void* inout, int* len, MPI_Datatype* dptr) {
    Node* input = (Node*)in;
    Node* output = (Node*)inout;
    Node tmp;
    for(int i = 0; i < *len; i++) {
        output[i] = (input[i].weight < output[i].weight) ? input[i] : output[i];
    }
}

void MPI_dijkstra::read_graph_data(vector<vector<Node>>& graph, int& rank, int& comm_size, std::pair<CV_INT, CV_INT>& range) {
    ifstream fin;
    fin.open(input_file);

    char flag;
    int delta = N / comm_size;
    CV_INT src, dst;
    CW_INT weight;
    string line;

    range.first = delta * rank;
    range.second = (rank != comm_size - 1) ? delta * (rank + 1) : N + 1;

    while(!fin.eof()) {
        getline(fin, line);
        if(line[0] != 'a') continue;

        istringstream ss(line);
        ss >> flag >> src >> dst >> weight;
        if(range.first <= dst && dst < range.second) 
            graph[src - 1].push_back({dst - 1, weight});
    }
}

void MPI_dijkstra::relax(vector<vector<Node>>& graph, vector<CW_INT>& tent, vector<bool>& visited, Node* latest_visited, pair<CV_INT, CV_INT>& range) {
    CV_INT recv_v = latest_visited->dst;
    CV_INT local_min_v = -1;                  // 다음 방문 가능한 노드 중 local minimum. (없으면 -1 반환)
    CW_INT local_min_w = CW_INT_MAX;

    // update vistied, tent
    visited[recv_v] = true;
    tent[recv_v] = latest_visited->weight;

    // 최근 vertex의 인접 노드들의 tent[] update
    for(CV_INT j = 0; j < graph[recv_v].size(); j++) {
        Node adj_node = graph[recv_v][j];
        if(visited[adj_node.dst]) continue;              
        tent[adj_node.dst] = MIN(tent[adj_node.dst], latest_visited->weight + adj_node.weight);
    }
    for(int i = range.first; i < range.second; i++) {
        if(!visited[i] && local_min_w > tent[i]) {
            local_min_v = i;
            local_min_w = tent[i];
        }
    }
    latest_visited->dst = local_min_v;
    latest_visited->weight = local_min_w;
}

void MPI_dijkstra::mpi_dijkstra_calc(int argc, char *argv[]) {
    int rank, comm_size;
    MPI_Init(&argc, &argv);                 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    vector<vector<Node>> graph(N);                              
    vector<CW_INT> tent(N, CW_INT_MAX);                         // distance vector
    vector<bool> visited(N, false);                             
    pair<CV_INT, CV_INT> range;                                 // local vertex range
    Node latest_visited;

    // define custom mpi type
    MPI_Datatype ctype;
    MPI_Type_contiguous(2, MPI_INT, &ctype);
    MPI_Type_commit(&ctype);

    // define custom operation
    MPI_Op cMax;
    MPI_Op_create(&myMax, 1, &cMax);

    read_graph_data(graph, rank, comm_size, range);                    

    if(rank == ROOT) {
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Rank %d Input Complete!\n", rank);

        using std::chrono::high_resolution_clock;
        using std::chrono::duration;
        auto t1 = high_resolution_clock::now();

        latest_visited.dst = startVertex;
        latest_visited.weight = 0;
        Node tmp;
        for(CV_INT i = 0; i < N; i++) {
            MPI_Bcast(&latest_visited, 1, ctype, ROOT, MPI_COMM_WORLD);

            if(latest_visited.dst == -1) {
                printf("Rank %d Finished at %d\n", rank, i);
                break; 
            }

            relax(graph, tent, visited, &latest_visited, range);        // latest_visited 재사용
            MPI_Reduce(&latest_visited, &tmp, 1, ctype, cMax, ROOT, MPI_COMM_WORLD);
            latest_visited = tmp;

            // log
            if(i % 5000 == 0) {
                printf("%d / %d\n", i, N);
            }
        }

        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;
        cout << "elapsed time : " << ms_double.count() << "ms\n";

        cout << "Calculate Complete!\n";


        ofstream fout;
        fout.open("./result_mpi_list.txt");
        for(CV_INT i = 0; i < N; i++) {
            if(tent[i] == CW_INT_MAX)
                fout << "Min dist " << startVertex << " to " << i + 1 << " = INF\n";
            else
                fout << "Min dist " << startVertex << " to " << i + 1 << " = " << tent[i] << '\n';
        }
    } else {
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Rank %d Input Complete!\n", rank);

        for(CV_INT i = 0; i < N; i++) {
            MPI_Bcast(&latest_visited, 1, ctype, ROOT, MPI_COMM_WORLD);
            if(latest_visited.dst == -1) {
                printf("Rank %d Finished at %d\n", rank, i);
                break;
            }

            relax(graph, tent, visited, &latest_visited, range);
            MPI_Reduce(&latest_visited, NULL, 1, ctype, cMax, ROOT, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}