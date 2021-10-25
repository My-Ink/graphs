#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

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
// Multiple edges
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

void find_bridges_dfs_(
    const Graph& g,
    vertex_t v,
    vertex_t parent,
    int& timer,
    std::vector<bool>& visited,
    std::vector<int>& time_up,
    std::vector<int>& time_in,
    std::vector<encoded_edge_t>& bridges
) {
    time_up[v] = time_in[v] = ++timer;
    visited[v] = true;
    for (auto u: g.get_neighbors(v)) {
        if (u == parent)
            continue;
        if (visited[u])
            time_up[v] = std::min(time_up[v], time_in[u]);
        else {
            find_bridges_dfs_(g, u, v, timer, visited, time_up, time_in, bridges);
            time_up[v] = std::min(time_up[v], time_up[u]);
            if (time_up[u] > time_in[v] && !g.is_multiple_edge(v, u)) {
                bridges.push_back(g.get_edge_encoder().encode(v, u));
            }
        }
    }
}

}

decltype(auto) find_bridges(const Graph& g) {
    int timer = 0;
    std::vector<bool> visited(g.n_vertices(), false);
    std::vector<int> time_up(g.n_vertices());
    std::vector<int> time_in(g.n_vertices());
    std::vector<encoded_edge_t> bridges;

    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (!visited[v])
            impl::find_bridges_dfs_(g, v, -1, timer, visited, time_up, time_in, bridges);
    }

    return bridges;
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
    const auto& edge_encoder = g.get_edge_encoder();
    std::unordered_map<vertex_t, int> edges_codes(n_edges);

    for (int i = 0; i < n_edges; ++i) {
        int from, to;
        cin >> from >> to;
        g.add_edge(from - 1, to - 1);
        edges_codes.emplace(edge_encoder.encode(from - 1, to - 1), i + 1);
    }

    auto bridges = find_bridges(g);
    cout << bridges.size() << '\n';
    for (auto& bridge: bridges)
        bridge = edges_codes[bridge];
    std::sort(bridges.begin(), bridges.end());
    for (auto bridge: bridges)
        cout << bridge << ' ';
}