#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std; 

struct Node {
    int dst;
    int weight;
};

int n;          // node 개수
int m;          // edge 개수

void dijkstra(vector<vector<Node>>& graph, int src, vector<int>& dist) {
    vector<bool> visited(n, false);
    
    dist[src] = 0;
    visited[src] = true;

    // src vertex 주변 weight init
    for(int j = 0; j < graph[src].size(); j++) {
        Node adj_node = graph[src][j];
        dist[adj_node.dst] = min(dist[adj_node.dst], adj_node.weight);
    }

    for(int i = 0; i < n - 1; i++) {
        // 방문하지 않은 노드 중 가장 가까운 노드
        int min_weight = INT32_MAX;
        int min_vertex;    
        for(int j = 0; j < n; j++) {
            if(!visited[j] && dist[j] < min_weight) {
                min_weight = dist[j];
                min_vertex = j;
            }
        }

        if(i % 500 == 0) {
            cout << i << " / " << n - 2 << ", weight = " << min_weight << '\n';
        }

        // 해당 노드를 visited로 바꾸고 dist update
        visited[min_vertex] = true;
        for(int j = 0; j < graph[min_vertex].size(); j++) {
            Node next_node = graph[min_vertex][j];              // 다음 Node
            if(visited[next_node.dst]) continue;
            dist[next_node.dst] = min(dist[next_node.dst], dist[min_vertex] + next_node.weight);
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
    fout.open("./result_list.txt");
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