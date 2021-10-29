#include <iostream>
#include <vector>
#include <unordered_set>

namespace graph
{
using vertex_t = int64_t;
using list_t = std::unordered_multiset<vertex_t>;

template<class T>
class PairEncoder
{
 public:
    using value_t = T;
    using pair_t = std::pair<value_t, value_t>;

    explicit PairEncoder(value_t key, bool ordered) : key_(key), ordered_(ordered) {}

    value_t encode(value_t first, value_t second) const {
        if (!ordered_ && first > second)
            std::swap(first, second);
        return first * key_ + second;
    }

    pair_t decode(value_t encoded) const {
        if (key_ == 0)
            return std::make_pair(0, 0);
        return std::make_pair(encoded / key_, encoded % key_);
    }

 private:
    value_t key_;
    bool ordered_;
};

using edge_encoder_t = PairEncoder<vertex_t>;
using encoded_edge_t = vertex_t;

// 0-indexing
class Graph
{
 public:
    virtual const list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;
    virtual bool is_multiple_edge(vertex_t from, vertex_t to) const = 0;

    const edge_encoder_t& get_edge_encoder() const {
        return edge_encoder;
    }

    size_t n_vertices() const {
        return n_vertices_;
    }

 protected:
    size_t n_vertices_;
    bool is_directed_;
    edge_encoder_t edge_encoder;

    Graph(size_t n_vertices, bool is_directed)
        : n_vertices_(n_vertices),
          is_directed_(is_directed),
          edge_encoder((vertex_t)n_vertices, is_directed) {}
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

    bool is_multiple_edge(vertex_t from, vertex_t to) const override {
        return (adj_lists_[from].count(to) > 1);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

void calc_time_ups_dfs_(
    const Graph& g,
    vertex_t v,
    vertex_t parent,
    int& timer,
    std::vector<bool>& visited,
    std::vector<int>& time_up,
    std::vector<int>& time_in
) {
    time_up[v] = time_in[v] = ++timer;
    visited[v] = true;
    for (auto u: g.get_neighbors(v)) {
        if (u == parent)
            continue;
        if (visited[u])
            time_up[v] = std::min(time_up[v], time_in[u]);
        else {
            calc_time_ups_dfs_(g, u, v, timer, visited, time_up, time_in);
            time_up[v] = std::min(time_up[v], time_up[u]);
        }
    }
}

void highlight_dcc_dfs_(
    const Graph& g,
    vertex_t v,
    int dcc_id,
    int& max_dcc_id,
    std::vector<int>& dcc_ids,
    const std::vector<int>& time_up,
    const std::vector<int>& time_in
) {
    dcc_ids[v] = dcc_id;
    for (auto u : g.get_neighbors(v)) {
        if (dcc_ids[u] == -1) {
            if (time_up[u] > time_in[v]) {
                ++max_dcc_id;
                highlight_dcc_dfs_(g, u, max_dcc_id, max_dcc_id, dcc_ids, time_up, time_in);
            } else {
                highlight_dcc_dfs_(g, u, dcc_id, max_dcc_id, dcc_ids, time_up, time_in);
            }
        }
    }
}

}

struct graph_highlights_t {
    int n_clusters;
    std::vector<int> ids;
};

// Double connectivity components
decltype(auto) highlight_all_dcc(const Graph& g) {
    int timer = 0;
    std::vector<bool> visited(g.n_vertices(), false);
    std::vector<int> time_up(g.n_vertices());
    std::vector<int> time_in(g.n_vertices());

    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (!visited[v])
            impl::calc_time_ups_dfs_(g, v, -1, timer, visited, time_up, time_in);
    }

    int max_id = 0;
    std::vector<int> dcc_ids(g.n_vertices(), -1);
    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (dcc_ids[v] == -1) {
            impl::highlight_dcc_dfs_(g, v, max_id, max_id, dcc_ids, time_up, time_in);
            ++max_id;
        }
    }

    return graph_highlights_t{max_id, dcc_ids};
}

int count_leaves_in_cluster_tree(const Graph& g, const graph_highlights_t& highlights) {
    int n_leaves = 0;
    std::vector<int> n_neighbors(highlights.n_clusters, 0);
    std::vector<bool> visited(g.n_vertices(), false);
    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (!visited[v]) {
            for (auto u : g.get_neighbors(v)) {
                if (highlights.ids[u] != highlights.ids[v])
                    ++n_neighbors[highlights.ids[v]];
            }
        }
    }
    for (auto k : n_neighbors) {
        if (k == 1)
            ++n_leaves;
    }
    return n_leaves;
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

    auto dcc_highlights = highlight_all_dcc(g);
    cout << (count_leaves_in_cluster_tree(g, dcc_highlights) + 1) / 2;
}
