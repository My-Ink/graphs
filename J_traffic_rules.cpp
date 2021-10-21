#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>

namespace graph
{
using vertex_t = int32_t;
using distance_t = int32_t;
using list_t = std::unordered_set<vertex_t>;
using vector_t = std::vector<vertex_t>;

// 0-indexing
class Graph
{
 public:
    virtual const list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;

    size_t size() const {
        return n_vertices;
    }

 protected:
    size_t n_vertices;
    bool is_directed_;

    Graph(size_t n, bool is_directed) : n_vertices(n), is_directed_(is_directed) {}
};

class AdjListsGraph : public Graph
{
 public:
    using adj_lists_t = std::vector<list_t>;

    AdjListsGraph(size_t n_vertices, bool is_directed)
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices) {}

    const list_t& get_neighbors(vertex_t v) const override {
        return adj_lists_[v];
    }

    void add_edge(vertex_t from, vertex_t to) override {
        adj_lists_[from].insert(to);
        if (!is_directed_)
            adj_lists_[to].insert(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

decltype(auto) find_shortest_paths_from_vertex_(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.size(), -1);
    std::vector<vertex_t> prev(g.size(), -1);

    std::queue<vertex_t> q;
    q.push(s);
    dist[s] = 0;

    while (!q.empty()) {
        auto v = q.front();
        for (auto u: g.get_neighbors(v)) {
            if (dist[u] == -1) {
                dist[u] = dist[v] + 1;
                prev[u] = v;
                q.push(u);
            }
        }
        q.pop();
    }
    return std::make_pair(dist, prev);
}

bool is_bipartite_impl_(
    const Graph& g,
    vertex_t v,
    bool color,
    std::vector<bool>& visited,
    std::vector<bool>& colors
) {
    visited[v] = true;
    colors[v] = color;
    for (auto u: g.get_neighbors(v)) {
        if (!visited[u]) {
            if (!is_bipartite_impl_(g, u, !color, visited, colors))
                return false;
        }
        else if (colors[u] == color) {
            return false;
        }
    }
    return true;
}

void top_sort_impl_(
    const Graph& g,
    vertex_t v,
    std::vector<bool>& viewed,
    vector_t::reverse_iterator& it
) {
    viewed[v] = true;
    for (auto u: g.get_neighbors(v)) {
        if (!viewed[u])
            top_sort_impl_(g, u, viewed, it);
    }
    *it = v;
    ++it;
}

void scc_impl_(
    const Graph& inv_g,
    vertex_t v,
    int component_id,
    std::vector<int>& components
) {
    components[v] = component_id;
    for (auto u: inv_g.get_neighbors(v)) {
        if (components[u] == -1)
            scc_impl_(inv_g, u, component_id, components);
    }
}

}

decltype(auto) find_shortest_path(const Graph& g, vertex_t from, vertex_t to) {
    auto[dist, prev] = impl::find_shortest_paths_from_vertex_(g, from);
    distance_t d = dist[to];
    std::vector<vertex_t> path(d + 1);
    vertex_t curr = to;
    for (int i = d; i >= 0; --i) {
        path[i] = curr;
        curr = prev[curr];
    }
    return path;
}

bool is_bipartite(const Graph& g) {
    std::vector<bool> visited, colors(g.size(), false);
    for (vertex_t v = 0; (size_t)v < g.size(); ++v) {
        visited.assign(g.size(), false);
        if (!visited[v]) {
            if (!impl::is_bipartite_impl_(g, v, true, visited, colors))
                return false;
        }
    }
    return true;
}

vector_t top_sort(const Graph& g) {
    std::vector<bool> viewed(g.size(), false);
    vector_t sorted(g.size());
    auto it = sorted.rbegin();
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        if (!viewed[v])
            impl::top_sort_impl_(g, v, viewed, it);
    }
    return sorted;
}

decltype(auto) build_condensation(const Graph& g) {
    auto inv_g = AdjListsGraph(g.size(), true);
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        for (auto u: g.get_neighbors(v))
            inv_g.add_edge(u, v);
    }
    auto sorted = top_sort(g);

    std::vector<int> components(g.size(), -1);
    int component_id = 0;
    for (auto v: sorted) {
        if (components[v] == -1) {
            impl::scc_impl_(inv_g, v, component_id, components);
            ++component_id;
        }
    }

    auto cond = AdjListsGraph(component_id, true);
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        for (auto u: g.get_neighbors(v)) {
            if (components[v] != components[u])
                cond.add_edge(components[v], components[u]);
        }
    }

    return std::make_pair(cond, components);
}

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
    cin >> n_vertices >> n_edges;

    Graph&& g = AdjListsGraph(n_vertices, true);
    for (int i = 0 ; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from - 1, to - 1);
    }

    auto[cond, comp] = build_condensation(g);
    WeightedGraph&& weighted_scc = WeightedAdjListsGraph(cond.size(), true);
    for (vertex_t v = 0; v < (vertex_t)cond.size(); ++v) {
        for (auto u : cond.get_neighbors(v)) {
            weighted_scc.add_edge(v, u, 0);
            weighted_scc.add_edge(u, v, 1);
        }
    }

    int n_requests;
    cin >> n_requests;
    for (int i = 0; i < n_requests; ++i) {
        int from, to;
        cin >> from >> to;
        auto penalty = find_shortest_distance(weighted_scc, comp[from - 1], comp[to - 1]);
        cout << (penalty == INT32_MAX ? -1 : penalty) << '\n';
    }
}