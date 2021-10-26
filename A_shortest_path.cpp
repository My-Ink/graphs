#include <iostream>
#include <vector>
#include <set>
#include <queue>

namespace graph
{

using vertex_t = int32_t;
using distance_t = int32_t;
using adj_list_t = std::vector<vertex_t>;

class Graph
{
 public:
    virtual const adj_list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;

    size_t n_vertices() const {
        return n_vertices_;
    }

 protected:
    size_t n_vertices_;
    bool is_directed_;

    Graph(size_t n, bool is_directed) : n_vertices_(n), is_directed_(is_directed) {}
};

class AdjListsGraph : public Graph
{
 public:
    using adj_lists_t = std::vector<adj_list_t>;

    AdjListsGraph(size_t n_vertices, bool is_directed)
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices + 1) {}

    const adj_list_t& get_neighbors(vertex_t v) const override {
        return adj_lists_[v];
    }

    void add_edge(vertex_t from, vertex_t to) override {
        adj_lists_[from].push_back(to);
        if (!is_directed_)
            adj_lists_[to].push_back(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

struct path_data_t {
    std::vector<distance_t> dist;
    std::vector<vertex_t> prev;
};

static constexpr distance_t INF_DISTANCE = -1;

decltype(auto) find_shortest_paths_from_vertex(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.n_vertices() + 1, INF_DISTANCE);
    std::vector<vertex_t> prev(g.n_vertices() + 1);

    std::queue<vertex_t> queue;
    queue.push(s);
    dist[s] = 0;

    while (!queue.empty()) {
        auto v = queue.front();
        for (auto u: g.get_neighbors(v)) {
            if (dist[u] == INF_DISTANCE) {
                dist[u] = dist[v] + 1;
                prev[u] = v;
                queue.push(u);
            }
        }
        queue.pop();
    }
    return path_data_t{dist, prev};
}

}

decltype(auto) find_shortest_path(const Graph& g, vertex_t from, vertex_t to) {
    auto[dist, prev] = impl::find_shortest_paths_from_vertex(g, from);
    distance_t distance = dist[to];
    std::vector<vertex_t> path(distance + 1);
    vertex_t curr = to;
    for (int i = distance; i >= 0; --i) {
        path[i] = curr;
        curr = prev[curr];
    }
    return path;
}

}
using std::cout;
using std::cin;

using namespace graph;

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n_vertices, n_edges;
    cin >> n_vertices >> n_edges;
    int start, finish;
    cin >> start >> finish;

    Graph&& g = AdjListsGraph(n_vertices, false);

    for (int i = 0; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from, to);
    }

    auto path = find_shortest_path(g, start, finish);
    if (path.empty()) {
        cout << -1;
    }
    else {
        cout << path.size() - 1 << '\n';
        for (auto vertex: path)
            cout << vertex << ' ';
    }
}