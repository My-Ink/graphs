#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_set>
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace graph
{

using vertex_t = int64_t;
using edge_t = int64_t;

class NotImplementedError : public std::runtime_error
{
 public:
    explicit NotImplementedError(const char* method_name)
        : std::runtime_error((std::string("NotImplementedError: ") + method_name).c_str()) {}
};

class GraphBase
{
 public:
    virtual void add_edge(vertex_t from, vertex_t to) = 0;
    virtual vertex_t add_vertex() = 0;

    virtual size_t degree(vertex_t v) const = 0;
    virtual edge_t edge_end(edge_t e) const = 0;

    virtual std::unordered_set<vertex_t> neighbors(vertex_t v) const = 0;

    virtual const std::unordered_set<vertex_t>& neighbors_by_reference(vertex_t) const {
        throw NotImplementedError("neighbors_by_reference");
    }

    virtual std::vector<edge_t> outgoing_edges(vertex_t) const = 0;

    virtual const std::vector<edge_t>& outgoing_edges_by_reference(vertex_t) const {
        throw NotImplementedError("neighbors_by_reference");
    }

    size_t n_vertices() const {
        return n_vertices_;
    }

    size_t n_edges() const {
        return n_edges_;
    }

    bool empty() const {
        return (n_edges_ == 0);
    }

 protected:
    size_t n_vertices_;
    size_t n_edges_;
    bool is_directed_;

    GraphBase(size_t n_vertices, bool is_directed)
        : n_vertices_(n_vertices),
          n_edges_(0),
          is_directed_(is_directed) {}
};

class Graph : public GraphBase
{
    using adj_list_t = std::vector<edge_t>;
    using adj_lists_t = std::vector<adj_list_t>;

 public:
    Graph(size_t n_vertices, bool is_directed)
        : GraphBase(n_vertices, is_directed), adj_lists_(n_vertices), edge_ends_() {}

    void add_edge(vertex_t from, vertex_t to) override {
        adj_lists_[from].push_back((edge_t)n_edges_++);
        edge_ends_.emplace_back(to);
        if (!is_directed_) {
            adj_lists_[to].push_back((edge_t)n_edges_++);
            edge_ends_.push_back(from);
        }
    }

    vertex_t add_vertex() override {
        auto new_vertex = (vertex_t)n_vertices_++;
        adj_lists_.emplace_back();
        return new_vertex;
    }

    size_t degree(vertex_t v) const override {
        return adj_lists_[v].size();
    }

    std::unordered_set<vertex_t> neighbors(vertex_t v) const override {
        std::unordered_set<vertex_t> neighbors;
        for (auto e: adj_lists_[v]) {
            neighbors.insert(edge_ends_[e]);
        }
        return neighbors;
    }

    std::vector<edge_t> outgoing_edges(vertex_t v) const override {
        return adj_lists_[v];
    }

    const std::vector<edge_t>& outgoing_edges_by_reference(vertex_t v) const override {
        return adj_lists_[v];
    }

    edge_t edge_end(edge_t e) const override {
        return edge_ends_[e];
    }

 private:
    adj_lists_t adj_lists_;
    std::vector<vertex_t> edge_ends_;
};

class FastNeighborsGraph : public Graph
{
    using neighbors_list_t = std::unordered_set<vertex_t>;
    using neighbors_data_t = std::vector<neighbors_list_t>;

 public:
    FastNeighborsGraph(size_t n_vertices, bool is_directed)
        : Graph(n_vertices, is_directed), neighbors_data_(n_vertices) {}

    void add_edge(vertex_t from, vertex_t to) override {
        Graph::add_edge(from, to);
        neighbors_data_[from].insert(to);
        if (!is_directed_)
            neighbors_data_[to].insert(from);
    }

    vertex_t add_vertex() override {
        auto new_vertex = Graph::add_vertex();
        neighbors_data_.emplace_back();
        return new_vertex;
    }

    std::unordered_set<vertex_t> neighbors(vertex_t v) const override {
        return neighbors_data_[v];
    }

    const std::unordered_set<vertex_t>& neighbors_by_reference(vertex_t v) const override {
        return neighbors_data_[v];
    }

 private:
    neighbors_data_t neighbors_data_;
};

using path_t = std::vector<vertex_t>;

namespace impl
{

vertex_t skip_isolated_(vertex_t max_v, const std::vector<bool>& isolated) {
    vertex_t v = 0;
    while (isolated[v] && v < max_v)
        ++v;
    return v;
}

}

bool is_connected_without_isolated(const GraphBase& g, const std::vector<bool>& isolated) {
    if (g.empty())
        return false;

    std::vector<bool> visited(g.n_vertices(), false);
    std::queue<vertex_t> queue;

    queue.push(impl::skip_isolated_((vertex_t)g.n_vertices(), isolated));

    while (!queue.empty()) {
        auto v = queue.front();
        visited[v] = true;
        for (auto u: g.neighbors_by_reference(v)) {
            if (!visited[u]) {
                queue.push(u);
            }
        }
        queue.pop();
    }

    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        if (!visited[v] && !isolated[v])
            return false;
    }
    return true;
}

path_t find_full_euler_path_if_exists(const GraphBase& g, const std::vector<bool>& isolated) {
    path_t path(0);

    if (!is_connected_without_isolated(g, isolated))
        return path;

    std::vector<size_t> deg_out(g.n_vertices(), 0);
    std::vector<size_t> deg_in(g.n_vertices(), 0);
    for (vertex_t v = 0; v < (vertex_t)g.n_vertices(); ++v) {
        deg_out[v] = g.degree(v);
        for (auto e: g.outgoing_edges_by_reference(v)) {
            ++deg_in[g.edge_end(e)];
        }
    }

    std::stack<vertex_t> stack;
    stack.push(impl::skip_isolated_((vertex_t)g.n_vertices(), isolated));

    while (!stack.empty()) {
        auto v = stack.top();
        if (deg_out[v] == 0) {
            path.push_back(v);
            stack.pop();
        }
        else {
            auto e = g.outgoing_edges_by_reference(v)[g.degree(v) - deg_out[v]];
            stack.push(g.edge_end(e));
            --deg_in[g.edge_end(e)];
            --deg_out[v];
        }
    }
    std::reverse(path.begin(), path.end());
    return path;
}

}

using std::cout;
using std::cin;

using namespace graph;

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    size_t n_paths, n_squares;
    cin >> n_paths >> n_squares;

    Graph&& g = FastNeighborsGraph(n_squares, true);
    std::vector<bool> isolated(n_squares, true);

    for (size_t i = 0; i < n_paths; ++i) {
        size_t path_len;
        cin >> path_len;
        vertex_t prev;
        cin >> prev;
        isolated[prev - 1] = false;
        for (size_t j = 0; j < path_len; ++j) {
            vertex_t next;
            cin >> next;
            isolated[next - 1] = false;
            g.add_edge(prev - 1, next - 1);
            prev = next;
        }
    }

    auto euler_path = find_full_euler_path_if_exists(g, isolated);
    cout << euler_path.size();
    if (!euler_path.empty()) {
        for (auto v: euler_path)
            cout << ' ' << v + 1;
    }
}