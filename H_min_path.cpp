#include <iostream>
#include <vector>
#include <set>
#include <queue>

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
    virtual vertex_t add_vertex() = 0;

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
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices) {}

    const adj_list_t& get_neighbors(vertex_t v) const override {
        return adj_lists_[v];
    }

    void add_edge(vertex_t from, vertex_t to) override {
        adj_lists_[from].push_back(to);
        if (!is_directed_)
            adj_lists_[to].push_back(from);
    }

    vertex_t add_vertex() override {
        adj_lists_.push_back(adj_list_t());
        return (vertex_t)n_vertices_++;
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

struct path_data_t {
    std::vector<distance_t> dist;
    std::vector<vertex_t> prev;
};

static constexpr distance_t INF_DISTANCE = -1;

decltype(auto) find_shortest_paths_from_vertex(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.n_vertices(), INF_DISTANCE);
    std::vector<vertex_t> prev(g.n_vertices());

    std::queue<vertex_t> queue;
    queue.push(s);
    dist[s] = 0;

    while (!queue.empty()) {
        auto v = queue.front();
        for (auto u: g.get_neighbors(v)) {
            if (dist[u] == INF_DISTANCE) {
                dist[u] = dist[v] + 1;
                prev[u] = v;
                queue.push(u);
            }
        }
        queue.pop();
    }
    return path_data_t{dist, prev};
}

}

distance_t find_shortest_distance(const Graph& g, vertex_t from, vertex_t to) {
    return impl::find_shortest_paths_from_vertex(g, from).dist[to];
}

void insert_chain(Graph& g, vertex_t begin, vertex_t end, size_t n_links) {
    if (n_links == 0) return;
    vertex_t prev = begin;
    while (n_links--) {
        auto link = g.add_vertex();
        g.add_edge(prev, link);
        prev = link;
    }
    g.add_edge(prev, end);
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
    --start, --finish;

    Graph&& g = AdjListsGraph(n_vertices, true);

    for (int i = 0; i < n_edges; ++i) {
        int from, to, weight;
        cin >> from >> to >> weight;
        --from, --to;
        if (weight > 1)
            insert_chain(g, from, to, weight - 1);
        else
            g.add_edge(from, to);
    }

    std::cout << find_shortest_distance(g, start, finish);
}