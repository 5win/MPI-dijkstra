#include <iostream>
#include <mpi.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

#define MAXX(x, y) (((x) > (y)) ? (x) : (y))
#define MINN(x, y) (((x) < (y)) ? (x) : (y))
#define N 264346
#define M 733846
#define ROOT 0

// Custom data type
#define CV_INT int              // Custom Vertex data type
#define CW_INT int
#define CW_INT_MAX INT32_MAX
#define CMPI_DTYPE MPI_INT      

// EDGE 가 맞는 표현인가?
struct Node {
    CV_INT dst;
    CW_INT weight;
};

void read_graph_data(vector<vector<Node>>& graph, int& rank, int& comm_size, pair<CV_INT, CV_INT>& range) {
    ifstream fin;
    fin.open("../USA-road-d.NY.txt");

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

void relax(vector<vector<Node>>& graph, vector<CW_INT>& tent, vector<bool>& visited, CV_INT* latest_visited, pair<CV_INT, CV_INT>& range) {
    CV_INT recv_v = latest_visited[0];
    CV_INT local_min_v = -1;                  // 다음 방문 가능한 노드 중 local minimum. (없으면 -1 반환)
    CW_INT local_min_w = CW_INT_MAX;

    // update vistied, tent
    visited[recv_v] = true;
    tent[recv_v] = latest_visited[1];

    // 최근 vertex의 인접 노드들의 tent[] update
    for(CV_INT j = 0; j < graph[recv_v].size(); j++) {
        Node adj_node = graph[recv_v][j];
        if(visited[adj_node.dst]) continue;              
        tent[adj_node.dst] = MINN(tent[adj_node.dst], latest_visited[1] + adj_node.weight);
    }
    for(int i = range.first; i < range.second; i++) {
        if(!visited[i] && local_min_w > tent[i]) {
            local_min_v = i;
            local_min_w = tent[i];
        }
    }
    latest_visited[0] = local_min_v;
    latest_visited[1] = local_min_w;
}

int main(int argc, char *argv[]) {

    // if(argc != 3) {
    //     cerr << "Please input file name, source vertex num!\n";
    //     return 1;
    // }     

    // ifstream fin;
    // fin.open(argv[1]);
    // int startVertex = stoi(argv[2]);
    // fin.open("../USA-road-d.NY.txt");

    int rank, comm_size;
    MPI_Init(&argc, &argv);                 // mpi option 주소 + 개수를 넘기면 되나?
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    vector<vector<Node>> graph(N);                              
    vector<CW_INT> tent(N, CW_INT_MAX);                         // distance vector
    vector<bool> visited(N, false);                             
    pair<CV_INT, CV_INT> range;                                 // local vertex range

    read_graph_data(graph, rank, comm_size, range);                    
    printf("Rank %d Input Complete!\n", rank);

    if(rank == ROOT) {
        CV_INT startVertex = 0;
        CV_INT latest_visited[2] = {startVertex, 0};            // start node info
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Status status;
        
        // TODO) 자료형을 struct로 바꾸고 MPI_BYTE 단위로 보내는 버전 해보기
        // Node latest_visited = {startVertex, tent[startVertex]};

        using std::chrono::high_resolution_clock;
        using std::chrono::duration;
        auto t1 = high_resolution_clock::now();

        for(CV_INT i = 0; i < N; i++) {
            MPI_Bcast(latest_visited, 2, CMPI_DTYPE, ROOT, MPI_COMM_WORLD);

            if(latest_visited[0] == -1) {
                printf("Rank %d Finished at %d\n", rank, i);
                break; 
            }
            relax(graph, tent, visited, latest_visited, range);        // latest_visited 재사용

            // choose global minimum
            CV_INT global_min_v = latest_visited[0];
            CW_INT global_min_w = latest_visited[1];

            for(int j = 0; j < comm_size - 1; j++) {
                MPI_Recv(latest_visited, 2, CMPI_DTYPE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if(global_min_w > latest_visited[1]) {
                    global_min_v = latest_visited[0];
                    global_min_w = latest_visited[1];
                }
            }
            // log
            if(i % 5000 == 0) {
                printf("%d / %d\n", i, N);
            }
            latest_visited[0] = global_min_v;
            latest_visited[1] = global_min_w;
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
        CV_INT latest_visited[2];

        for(CV_INT i = 0; i < N; i++) {
            MPI_Bcast(latest_visited, 2, CMPI_DTYPE, ROOT, MPI_COMM_WORLD);
            if(latest_visited[0] == -1) {
                printf("Rank %d Finished at %d\n", rank, i);
                break;
            }

            relax(graph, tent, visited, latest_visited, range);
            MPI_Send(latest_visited, 2, CMPI_DTYPE, ROOT, 0, MPI_COMM_WORLD);       // Send local min value
        }
    }

    MPI_Finalize();

    return 0;
}