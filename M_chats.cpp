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
    for (int i = 0 ; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from - 1, to - 1);
    }

    auto[cond, comp] = build_condensation(g);
    cout << cond.size() << '\n';

    std::vector<std::vector<vertex_t>> comp_lists(cond.size());
    for (vertex_t v = 0; v < (vertex_t)g.size(); ++v)
        comp_lists[comp[v]].push_back(v);
    for (auto& comp_list : comp_lists) {
        cout << comp_list.size() << '\n';
        for (auto s : comp_list)
            cout << s + 1 << ' ';
        cout << '\n';
    }
}