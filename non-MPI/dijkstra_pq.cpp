#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>

using namespace std; 

struct Node {
    int dst;
    int weight;
};

int n;          // node 개수
int m;          // edge 개수
int step;

void dijkstra(vector<vector<Node>>& graph, int src, vector<int>& dist) {
    // 시각화용 

    priority_queue<pair<int, int>> pq;

    pq.push({0, src});
    dist[src] = 0;

    int push_cnt = 1;
    while(!pq.empty()) {


        int distance = -pq.top().first;
        int cur_node = pq.top().second;
        pq.pop();

        if(dist[cur_node] < distance) continue;     // 이미 최단경로인 경우 continue
        
        if(push_cnt % 1000 == 0) {
            cout << "Step " << ++step << ") " << push_cnt << " / " << n << ", pq size = " << pq.size() << '\n';
        }

        for(int i = 0; i < graph[cur_node].size(); i++) {
            int cost = distance + graph[cur_node][i].weight;
            if(cost < dist[graph[cur_node][i].dst]) {
                dist[graph[cur_node][i].dst] = cost;
                pq.push(make_pair(-cost, graph[cur_node][i].dst));
                push_cnt++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        cerr << "Please input source vertex number!\n";
        return 1;
    }

    ifstream fin;
    fin.open("../USA-road-d.NY.txt");

    // int n = stoi(argv[1]);
    n = 264346;
    m = 733846;
    int startVertex = stoi(argv[1]);
    vector<vector<Node>> graph(n);
    vector<int> dist(n, INT32_MAX);

    char flag;
    int src, dst, weight;
    string line;
    while(!fin.eof()) {
        getline(fin, line);
        if(line[0] != 'a') continue;                // egde 데이터 아니면 continue

        istringstream ss(line);
        ss >> flag >> src >> dst >> weight;

        graph[src - 1].push_back({dst - 1, weight});
    }

    cout << "Input complete!\n";

    dijkstra(graph, startVertex - 1, dist);

    cout << "Calculate complete!\n";


    ofstream fout;
    fout.open("./result_pq.txt");
    // for(int i = 0; i < graph.size(); i++) {
    for(int i = 0; i < graph.size(); i++) {
        // printf("Minimum dist %d to %d = %d\n", startVertex, i + 1, dist[i]);
        if(dist[i] == INT32_MAX)
            fout << "Minimum dist " << startVertex << " to " << i + 1 << " = INF\n";
        else
            fout << "Minimum dist " << startVertex << " to " << i + 1 << " = " << dist[i] << '\n';
    }

    cout << "Finished!\n";

    return 0;
}