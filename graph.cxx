#include <iostream>
#include <vector>
#include <queue>

struct edge
{
	int dest;
	int cost;
	edge(int dest, int cost) {
		this->dest = dest;
		this->cost = cost;
	}
};

void connect(std::vector<std::vector<edge>> &graph, int src, int dest, int cost)
{
	graph[src].push_back(edge(dest, cost));
	graph[dest].push_back(edge(src, cost));
}

void breadth_first_search(std::vector<std::vector<edge>> &graph, int start)
{
	std::vector<bool> visited(graph.size());
	std::fill(std::begin(visited), std::end(visited), false);
	std::queue<int> queue;
	queue.push(start);
	while (!queue.empty()) {
		int u = queue.front();
		queue.pop();
		for (edge v: graph[u]) {
			if (!visited[v.dest]) {
				std::cout << v.dest << char(9);
				queue.push(v.dest);
			}
		}
	}
}

void breadth_first_search_shortest_path(std::vector<std::vector<edge>> &graph, int start)
{
	std::vector<int> dist(graph.size(), -1);
	std::queue<int> q;
	q.push(start);
	dist[start] = 0;
	while (!q.empty()) {
		int u = q.front();
		q.pop();
		for (edge v: graph[u])
			if (dist[v.dest] < 0) {
				dist[v.dest] = dist[u] + 1;
				q.push(v.dest);
			}
	}
}

void connect(std::vector<std::vector<int>> &graph, int src, int dest)
{
	graph[src].push_back(dest);
	graph[dest].push_back(src);
}

void breadth_first_search(std::vector<std::vector<int>> &graph, int start)
{
	std::vector<bool> visited(graph.size(), false);
	std::queue<int> q;
	q.push(start);
	while (!q.empty()) {
		int u = q.front();
		q.pop();
		for (int v: graph[u])
			if (!visited[v.dest]) {
				std::cout << v.dest << char(9);
				q.push(v.dest);
			}
	}
}

void breadth_first_search_shortest_path(std::vector<std::vector<int>> &graph, int start)
{
	std::vector<int> dist(graph.size(), -1);
	std::queue<int> q;
	q.push(start);
	dist[start] = 0;
	while (!q.empty()) {
		int u = q.front();
		q.pop();
		for (int v: graph[u])
			if (dist[v] < 0) {
				dist[v] = dist[u] + 1;
				q.push(v);
			}
	}
}

int main()
{
	std::vector<std::vector<edge>> graph(6);
	connect(graph, 0, 1, 1);
	connect(graph, 1, 2, 1);
	connect(graph, 1, 3, 1);
	connect(graph, 2, 3, 1);
	connect(graph, 2, 5, 1);
	connect(graph, 4, 5, 1);
	breadth_first_search(graph, 0);	
}
