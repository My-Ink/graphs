#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>

namespace graph
{
using vertex_t = int32_t;
using list_t = std::unordered_set<vertex_t>;

// 0-indexing
class Graph
{
 public:
    virtual const list_t& get_neighbors(vertex_t v) const = 0;
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

void find_cut_points_dfs_(
    const Graph& g,
    vertex_t v,
    vertex_t parent,
    int& timer,
    std::vector<bool>& visited,
    std::vector<int>& time_up,
    std::vector<int>& time_in,
    std::set<vertex_t>& cut_points
) {
    time_up[v] = time_in[v] = ++timer;
    visited[v] = true;
    int n_children = 0;
    for (auto u: g.get_neighbors(v)) {
        if (u == parent)
            continue;
        if (visited[u])
            time_up[v] = std::min(time_up[v], time_in[u]);
        else {
            find_cut_points_dfs_(g, u, v, timer, visited, time_up, time_in, cut_points);
            time_up[v] = std::min(time_up[v], time_up[u]);
            if (parent != -1 && time_up[u] >= time_in[v]) {
                cut_points.insert(v);
            }
            ++n_children;
        }
    }
    if (parent == -1 && n_children > 1)
        cut_points.insert(v);
}

}

decltype(auto) find_cut_points(const Graph& g) {
    int timer = 0;
    std::vector<bool> visited(g.n_vertices(), false);
    std::vector<int> time_up(g.n_vertices());
    std::vector<int> time_in(g.n_vertices());
    std::set<vertex_t> cut_points;

    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (!visited[v])
            impl::find_cut_points_dfs_(g, v, -1, timer, visited, time_up, time_in, cut_points);
    }

    return cut_points;
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

    Graph&& g = AdjListsGraph(n_vertices, false);
    for (int i = 0; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from - 1, to - 1);
    }

    auto cut_points = find_cut_points(g);
    cout << cut_points.size() << '\n';
    for (auto cut_point: cut_points)
        cout << cut_point + 1 << '\n';
}