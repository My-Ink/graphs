#include <iostream>
#include <vector>
#include <set>
#include <queue>

namespace graph
{

using vertex_t = int32_t;
using distance_t = int32_t;
using list_t = std::set<vertex_t>;

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
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices + 1) {}

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

decltype(auto) find_shortest_paths_from_vertex(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.size() + 1, -1);
    std::vector<vertex_t> prev(g.size() + 1, -1);

    std::queue<vertex_t> q;
    q.push(s);
    dist[s] = 0;

    while (!q.empty()) {
        auto v = q.front();
        for (auto u: g.get_neighbors(v)) {
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

bool is_bipartite_impl_(
    const Graph& g,
    vertex_t v,
    bool color,
    std::vector<bool>& visited,
    std::vector<bool>& colors
) {
    visited[v] = true;
    colors[v] = color;
    for (auto u: g.get_neighbors(v)) {
        if (!visited[u]) {
            if (!is_bipartite_impl_(g, u, !color, visited, colors))
                return false;
        }
        else if (colors[u] == color) {
            return false;
        }
    }
    return true;
}

}

decltype(auto) find_shortest_path(const Graph& g, vertex_t from, vertex_t to) {
    auto[dist, prev] = impl::find_shortest_paths_from_vertex(g, from);
    distance_t d = dist[to];
    std::vector<vertex_t> path(d + 1);
    vertex_t curr = to;
    for (int i = d; i >= 0; --i) {
        path[i] = curr;
        curr = prev[curr];
    }
    return path;
}

bool is_bipartite(const Graph& g) {
    std::vector<bool> visited, colors(g.size() + 1, false);
    for (vertex_t v = 1; (size_t)v < g.size() + 1; ++v) {
        visited.assign(g.size() + 1, false);
        if (!visited[v]) {
            if (!impl::is_bipartite_impl_(g, v, true, visited, colors))
                return false;
        }
    }
    return true;
}

}

using namespace graph;

using std::cout;
using std::cin;

class ManhattanGraph : public AdjListsGraph
{
 public:
    ManhattanGraph(int n, int m, const std::vector<bool>& has_sub)
        : AdjListsGraph(n * m, false), distances_(n * m, INT32_MAX) {
        std::vector<int> subs;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                vertex_t v = i * m + j;
                if (j < m - 1)
                    add_edge(v, v + 1);
                if (i < n - 1)
                    add_edge(v, v + m);
                if (has_sub[v]) {
                    distances_[v] = 0;
                    subs.push_back(v);
                }
            }
        }
        for (auto& s : subs)
            find_nearest_crossroads_bfs_(s);
    }

    decltype(auto) get_distances() const {
        return distances_;
    }

 private:
    std::vector<int> distances_;

    void find_nearest_crossroads_bfs_(vertex_t sub) {
        std::queue<vertex_t> q;
        q.push(sub);
        q.push(-1);

        int d = 1;
        while (q.front() != -1) {
            auto v = q.front();
            for (auto u: get_neighbors(v)) {
                if (distances_[u] > d) {
                    distances_[u] = d;
                    q.push(u);
                }
            }
            q.pop();
            if (q.front() == -1) {
                q.pop();
                q.push(-1);
                ++d;
            }
        }
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    std::vector<bool> subs_indicators(n * m, false);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            bool b;
            cin >> b;
            subs_indicators[i * m + j] = b;
        }
    }

    auto m_graph = ManhattanGraph(n, m, subs_indicators);
    auto&& distances = m_graph.get_distances();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j)
            cout << distances[i * m + j] << ' ';
        cout << '\n';
    }
}