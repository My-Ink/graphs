#include <iostream>
#include <vector>
#include <set>
#include <queue>

namespace graph
{

using vertex_t = int32_t;
using distance_t = int32_t;
using list_t = std::set<vertex_t>;

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
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices + 1) {}

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

decltype(auto) find_shortest_paths_from_vertex(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.size() + 1, -1);
    std::vector<vertex_t> prev(g.size() + 1, -1);

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

}

decltype(auto) find_shortest_path(const Graph& g, vertex_t from, vertex_t to) {
    auto[dist, prev] = impl::find_shortest_paths_from_vertex(g, from);
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
    std::vector<bool> visited, colors(g.size() + 1, false);
    for (vertex_t v = 1; (size_t)v < g.size() + 1; ++v) {
        visited.assign(g.size() + 1, false);
        if (!visited[v]) {
            if (!impl::is_bipartite_impl_(g, v, true, visited, colors))
                return false;
        }
    }
    return true;
}

}

using namespace graph;

using std::cout;
using std::cin;

bool is_correct(int number) {
    if (number < 1111 || number > 9999)
        return false;
    for (int b = 10; b < 10000; b *= 10) {
        if (number % 10 == 0) {
            return false;
        }
    }
    return true;
}

int shift_left(int number) {
    return (number % 1000) * 10 + number / 1000;
}

int shift_right(int number) {
    return (number % 10) * 1000 + number / 10;
}

AdjListsGraph fill_graph() {
    AdjListsGraph numbers_graph(9999, true);
    for (int i = 1111; i < 10000; ++i) {
        if (!is_correct(i)) continue;
        if (i < 9111)
            numbers_graph.add_edge(i, i + 1000);
        if (i % 10 > 1)
            numbers_graph.add_edge(i, i - 1);
        numbers_graph.add_edge(i, shift_left(i));
        numbers_graph.add_edge(i, shift_right(i));
    }
    return numbers_graph;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    auto g = fill_graph();
    int start, finish;
    cin >> start >> finish;
    auto path = find_shortest_path(g, start, finish);
    cout << path.size() << '\n';
    for (auto n : path)
        cout << n << '\n';
}