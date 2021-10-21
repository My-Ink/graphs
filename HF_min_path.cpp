#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>

namespace graph
{
using vertex_t = int32_t;
using weight_t = int32_t;
using wlist_t = std::unordered_map<vertex_t, weight_t>;

class WeightedGraph
{
 public:
    virtual const wlist_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to, weight_t weight) = 0;

    size_t size() const {
        return n_vertices;
    }

 protected:
    size_t n_vertices;
    bool is_directed_;

    WeightedGraph(size_t n, bool is_directed) : n_vertices(n), is_directed_(is_directed) {}
};

class WeightedAdjListsGraph : public WeightedGraph
{
 public:
    using adj_lists_t = std::vector<wlist_t>;

    WeightedAdjListsGraph(size_t n_vertices, bool is_directed)
        : WeightedGraph(n_vertices, is_directed), adj_lists_(n_vertices) {}

    const wlist_t& get_neighbors(vertex_t v) const override {
        return adj_lists_[v];
    }

    void add_edge(vertex_t from, vertex_t to, weight_t weight) override {
        adj_lists_[from].emplace(to, weight);
        if (!is_directed_)
            adj_lists_[to].emplace(to, weight);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

// Dijkstra's Algorithm
decltype(auto) find_shortest_distances_from_vertex_(const WeightedGraph& graph, vertex_t vertex) {
    std::priority_queue<std::pair<weight_t, vertex_t>> q;
    std::vector<weight_t> dist(graph.size(), INT32_MAX);
    std::vector<bool> proc(graph.size(), false);

    dist[vertex] = 0;
    q.emplace(0, vertex);
    while (!q.empty()) {
        auto v = q.top().second;
        q.pop();
        if (proc[v])
            continue;
        proc[v] = true;

        for (auto vw_pair: graph.get_neighbors(v)) {
            auto[u, w] = vw_pair;
            if (dist[v] + w < dist[u]) {
                dist[u] = dist[v] + w;
                q.emplace(-dist[u], u);
            }
        }
    }

    return dist;
}

}

weight_t find_shortest_distance(const WeightedGraph& g, vertex_t from, vertex_t to) {
    return impl::find_shortest_distances_from_vertex_(g, from)[to];
}

}

using namespace graph;

using std::cout;
using std::cin;

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n_vertices, n_edges;
    vertex_t start, finish;

    cin >> n_vertices >> n_edges;
    cin >> start >> finish;

    WeightedGraph&& g = WeightedAdjListsGraph(n_vertices, true);
    for (int i = 0; i < n_edges; ++i) {
        vertex_t u, v;
        weight_t w;
        cin >> u >> v >> w;
        g.add_edge(u - 1, v - 1, w);
    }

    auto d = find_shortest_distance(g, start - 1, finish - 1);
    cout << (d == INT32_MAX ? -1 : d);
}