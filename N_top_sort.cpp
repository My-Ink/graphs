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

// Return true if graph has a cycle
bool top_sort_impl_(
    const Graph& g,
    vertex_t v,
    std::vector<int>& color,
    vector_t::reverse_iterator& it
) {
    color[v] = 1;
    for (auto u: g.get_neighbors(v)) {
        if (color[u] == 1) {
            return true;
        }
        else if (color[u] == 0) {
            if (top_sort_impl_(g, u, color, it))
                return true;
        }
    }
    color[v] = 2;
    *it = v;
    ++it;
    return false;
}

}

vector_t top_sort(const Graph& g) {
    std::vector<int> color(g.size(), false);
    vector_t sorted(g.size());
    auto it = sorted.rbegin();
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        if (color[v] == 0) {
            if (impl::top_sort_impl_(g, v, color, it)) {
                sorted.resize(0);
                break;
            }
        }
    }
    return sorted;
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

    auto sorted = top_sort(g);
    if (sorted.empty()) {
        cout << -1;
    }
    else {
        for (auto v: sorted)
            cout << v + 1 << ' ';
    }
}