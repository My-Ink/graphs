#include <iostream>
#include <vector>
#include <unordered_set>

namespace graph
{
using vertex_t = int32_t;
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

bool find_cycle_impl_(
    const Graph& g,
    vertex_t v,
    std::vector<int>& colors,
    std::vector<vertex_t>& prev,
    vertex_t& cycle_start
) {
    colors[v] = 1;
    bool ans = false;
    for (auto u : g.get_neighbors(v)) {
        if (colors[u] == 1) {
            prev[u] = v;
            cycle_start = u;
            return true;
        }
        if (colors[u] == 0) {
            prev[u] = v;
            ans = find_cycle_impl_(g, u, colors, prev, cycle_start);
        }
    }
    colors[v] = 2;
    return ans;
}

}

decltype(auto) find_cycle(const Graph& g) {
    std::vector<int> colors(g.size(), 0);
    std::vector<vertex_t> prev(g.size(), -1);
    vertex_t cycle_start = -1;
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        if (impl::find_cycle_impl_(g, v, colors, prev, cycle_start))
            break;
    }
    std::vector<vertex_t> cycle;
    if (cycle_start == -1)
        return cycle;
    cycle.push_back(cycle_start);
    auto v = prev[cycle_start];
    while (v != cycle_start) {
        cycle.push_back(v);
        v = prev[v];
    }
    return cycle;
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
    for (int i = 0; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from - 1, to - 1);
    }

    auto cycle = find_cycle(g);
    if (cycle.empty())
        cout << "NO\n";
    else {
        cout << "YES\n";
        for (auto it = cycle.rbegin(); it < cycle.rend(); ++it)
            cout << *it + 1 << ' ';
    }
}