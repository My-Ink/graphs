#include <iostream>
#include <vector>
#include <queue>
#include <tuple>

namespace graph
{

using vertex_t = int32_t;
using distance_t = int32_t;
using adj_list_t = std::vector<vertex_t>;

class Graph
{
 public:
    virtual const adj_list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;

    size_t size() const {
        return n_vertices_;
    }

 protected:
    size_t n_vertices_;
    bool is_directed_;

    Graph(size_t n_vertices, bool is_directed) : n_vertices_(n_vertices), is_directed_(is_directed) {}
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
        adj_lists_[from].push_back(to);
        if (!is_directed_)
            adj_lists_[to].push_back(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

template<template<class> class Container>
decltype(auto) find_shortest_paths_from_vertices(const Graph& graph, Container<vertex_t> init_vertices) {
    std::vector<distance_t> dist(graph.size() + 1, -1);
    std::vector<vertex_t> prev(graph.size() + 1, -1);

    std::queue<vertex_t> q;
    for (auto init_vertex : init_vertices) {
        q.push(init_vertex);
        dist[init_vertex] = 0;
    }

    while (!q.empty()) {
        auto v = q.front();
        for (auto u: graph.get_neighbors(v)) {
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

}

}

using namespace graph;

using std::cout;
using std::cin;

inline static constexpr vertex_t encode_pair(int first, int second, int key) {
    return first * key + second;
}

class ManhattanGraph : public AdjListsGraph
{
 public:
    ManhattanGraph(int n, int m, const std::vector<bool>& has_sub)
        : AdjListsGraph(n * m, false), distances_(n * m, INT32_MAX) {
        std::vector<int> subs;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                vertex_t v = encode_pair(i, j, m);
                vertex_t right_neighbor = encode_pair(i, j + 1, m);
                vertex_t bottom_neighbor = encode_pair(i + 1, j, m);
                if (j < m - 1)
                    dynamic_cast<AdjListsGraph*>(this)->add_edge(v, right_neighbor);
                if (i < n - 1)
                    dynamic_cast<AdjListsGraph*>(this)->add_edge(v, bottom_neighbor);
                if (has_sub[v]) {
                    distances_[v] = 0;
                    subs.push_back(v);
                }
            }
        }

        auto&&[dist, prev] = impl::find_shortest_paths_from_vertices(*this, subs);
        distances_ = std::move(dist);
    }

    const auto& get_distances() const {
        return distances_;
    }

 private:
    std::vector<int> distances_;
};

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    std::vector<bool> subs_indicators(n * m, false);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            bool is_sub;
            cin >> is_sub;
            subs_indicators[encode_pair(i, j, m)] = is_sub;
        }
    }

    auto m_graph = ManhattanGraph(n, m, subs_indicators);
    auto&& distances = m_graph.get_distances();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j)
            cout << distances[encode_pair(i, j, m)] << ' ';
        cout << '\n';
    }
}