
#include <iostream>
#include <vector>
#include <fstream>
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
        dist[graph[src][j].dst] = graph[src][j].weight;
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

        // 해당 노드를 visited로 바꾸고 dist update
        visited[min_vertex] = true;
        for(int j = 0; j < graph[min_vertex].size(); j++) {
            Node next_node = graph[min_vertex][j];              // 다음 Node
            dist[next_node.dst] = min(dist[next_node.dst], dist[min_vertex] + next_node.weight);
        }
    }
}

int main(int argc, char *argv[]) {
    cin.tie(NULL);
    ios_base::sync_with_stdio(false);  

    int startVertex;

    cin >> n >> m >> startVertex;
    startVertex--;

    vector<vector<Node>> graph(n);
    vector<int> dist(n, INT32_MAX);

    int src, dst, weight;
    for(int i = 0; i < m; i++) {
        cin >> src >> dst >> weight;
        graph[src - 1].push_back({dst - 1, weight});
    }

    dijkstra(graph, startVertex, dist);

    for(int i = 0; i < graph.size(); i++) {
        if(dist[i] == INT32_MAX)    cout << "INF\n";
        else                        cout << dist[i] << '\n';
    }

    return 0;
}