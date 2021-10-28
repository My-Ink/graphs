#include <iostream>
#include <vector>
#include <unordered_set>

namespace graph
{

using vertex_t = int32_t;
using adj_list_t = std::unordered_multiset<vertex_t>;

class Graph
{
 public:
    virtual const adj_list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;
    virtual void delete_edge(vertex_t from, vertex_t to) = 0;

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
        adj_lists_[from].insert(to);
        if (!is_directed_)
            adj_lists_[to].insert(from);
    }

    void delete_edge(vertex_t from, vertex_t to) override {
        adj_lists_[from].erase(to);
        if (!is_directed_)
            adj_lists_[to].erase(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

void euler_dfs_(Graph& g, vertex_t v, vertex_t parent, std::vector<vertex_t>& path) {
    for (auto u : g.get_neighbors(v)) {
        g.delete_edge(v, u);
        euler_dfs_(g, v, u, path);
    }
    path.push_back(v);
}

}

decltype(auto) find_euler_cycle_if_exists(Graph& g) {
    std::vector<vertex_t> path;
    path.push_back(0);
    impl::euler_dfs_(g, 0, path);
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


}