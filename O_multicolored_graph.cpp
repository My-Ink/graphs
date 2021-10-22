#include <iostream>
#include <vector>
#include <unordered_set>

namespace graph
{
using vertex_t = int32_t;
using list_t = std::vector<vertex_t>;

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
        adj_lists_[from].push_back(to);
        if (!is_directed_)
            adj_lists_[to].push_back(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

bool has_cycle_impl_(
    const Graph& g,
    vertex_t v,
    std::vector<int>& colors
) {
    colors[v] = 1;
    for (auto u : g.get_neighbors(v)) {
        if (colors[u] == 1)
            return true;
        if (colors[u] == 0 && has_cycle_impl_(g, u, colors))
            return true;
    }
    colors[v] = 2;
    return false;
}

}

decltype(auto) has_cycle(const Graph& g) {
    std::vector<int> colors(g.size(), 0);
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v) {
        if (colors[v] == 0 && impl::has_cycle_impl_(g, v, colors))
            return true;
    }
    return false;
}

}

using namespace graph;

using std::cout;
using std::cin;

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n_vertices;
    cin >> n_vertices;

    Graph&& g = AdjListsGraph(n_vertices, true);
    std::string colors;
    for (int i = 0; i < n_vertices - 1; ++i) {
        cin >> colors;
        int next = i + 1;
        for (char c : colors) {
            if (c == 'R')
                g.add_edge(i, next);
            else
                g.add_edge(next, i);
            ++next;
        }
    }

    cout << (has_cycle(g) ? "NO\n" : "YES\n");
}