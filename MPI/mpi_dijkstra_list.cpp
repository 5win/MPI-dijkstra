#include <iostream>
#include <mpi.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

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

void read_graph_data(vector<vector<Node>>& graph, int& rank, int& comm_size) {
    ifstream fin;
    fin.open("../USA-road-d.NY.txt");

    char flag;
    int delta = N / comm_size;
    CV_INT src, dst;
    CW_INT weight;
    string line;
    while(!fin.eof()) {
        getline(fin, line);
        if(line[0] != 'a') continue;

        istringstream ss(line);
        ss >> flag >> src >> dst >> weight;
        if(rank != comm_size - 1) {
            if(delta * rank <= dst && dst < delta * (rank + 1)) 
                graph[src - 1].push_back({dst - 1, weight});
        } else {        // 마지막 rank일 때는 남은 모든 노드를 관리해야함
            if(delta * rank <= dst && dst < N)
                graph[src - 1].push_back({dst - 1, weight});
        }
    }
    // printf("Rank %d) [%d, %d)\n", rank, delta * rank, delta * (rank + 1));
}

void relax(vector<vector<Node>>& graph, vector<CW_INT>& tent, vector<bool>& visited, CV_INT* latest_visited) {
    CV_INT recv_v = latest_visited[0];
    CV_INT local_min_v = 0;                  // 다음 방문 가능한 노드 중 local minimum. (없으면 -1 반환)
    CW_INT local_min_w = CW_INT_MAX;

    // update vistied, tent
    visited[recv_v] = true;
    tent[recv_v] = latest_visited[1];

    // 최근 vertex의 인접 노드들의 tent[] update
    for(CV_INT j = 0; j < graph[recv_v].size(); j++) {
        Node adj_node = graph[recv_v][j];
        if(visited[adj_node.dst]) continue;                     // 방문했으면 skip
        tent[adj_node.dst] = min(tent[adj_node.dst], latest_visited[1] + adj_node.weight);      // min 개선하기
        if(local_min_w > tent[adj_node.dst]) {
            local_min_v = adj_node.dst;
            local_min_w = tent[local_min_v];
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

    // init MPI
    int rank, comm_size;
    MPI_Init(&argc, &argv);                 // mpi option 주소 + 개수를 넘기면 되나?
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    vector<vector<Node>> graph(N);                              // graph 데이터 adjacency list -> dst, src, weight
    vector<CW_INT> tent(N, CW_INT_MAX);                         // distance vector
    vector<bool> visited(N, false);                     // local size로하면 너무 헷갈림

    read_graph_data(graph, rank, comm_size);                    // dst를 기준으로 graph 데이터 받아옴

    printf("Rank %d Input Complete!\n", rank);
    
    // CV_INT local_vertex_size = reverse_graph.size();         // 현재 rank에 할당된 vertex size

    if(rank == ROOT) {
        // start vertex init
        CV_INT startVertex = 0;
        // vector<bool> completed(comm_size, false);               // 각 rank의 작업이 끝났나?

        // broadcast : {u, tent(u)}
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Status status;
        // TODO) 자료형을 struct로 바꾸고 MPI_BYTE 단위로 보내는 버전 해보기
        CV_INT latest_visited[2] = {startVertex, 0};            // start node info
        // Node latest_visited = {startVertex, tent[startVertex]};

        for(CV_INT i = 0; i < N; i++) {
            MPI_Bcast(latest_visited, 2, CMPI_DTYPE, ROOT, MPI_COMM_WORLD);

            relax(graph, tent, visited, latest_visited);        // latest_visited 재사용

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
            // if(i % 5000 == 0) {
            //     printf("%d, %d\n", global_min_v, global_min_w);
            // }

            if(global_min_v != 0) {
                visited[global_min_v] = true;               // update global visited
                tent[global_min_v] = global_min_w;          // update global tent
            }

            latest_visited[0] = global_min_v;
            latest_visited[1] = global_min_w;
        }

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

            // if(i < 10) {
            //     printf("Rank %d/%d) %d, %d\n", rank, i, latest_visited[0], latest_visited[1]);
            // }

            relax(graph, tent, visited, latest_visited);

            // if(i < 10) {
            //     printf("Rank %d/%d) %d, %d\n", rank, i, latest_visited[0], latest_visited[1]);
            // }
            // latest_visited 에 local min info 저장
            MPI_Send(latest_visited, 2, CMPI_DTYPE, ROOT, 0, MPI_COMM_WORLD);       // Send local min value
        }
    }

    cout << "Finished!\n";
    MPI_Finalize();

    return 0;
}