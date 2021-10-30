#include <iostream>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <cstring>

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

class Graph
{
 public:
    virtual void add_edge(vertex_t from, vertex_t to) = 0;
    virtual vertex_t add_vertex() = 0;

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

 protected:
    size_t n_vertices_;
    size_t n_edges_;
    bool is_directed_;

    Graph(size_t n_vertices, bool is_directed)
        : n_vertices_(n_vertices),
          n_edges_(0),
          is_directed_(is_directed) {}
};

class AdjListsGraph : public Graph
{
    using adj_list_t = std::vector<edge_t>;
    using adj_lists_t = std::vector<adj_list_t>;

 public:
    AdjListsGraph(size_t n_vertices, bool is_directed)
        : Graph(n_vertices, is_directed), adj_lists_(n_vertices), edge_ends_() {}

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

    edge_t get_edge_end(edge_t e) const {
        return edge_ends_[e];
    }

 private:
    adj_lists_t adj_lists_;
    std::vector<vertex_t> edge_ends_;
};

class FastNeighborsGraph : public AdjListsGraph
{
    using neighbors_list_t = std::unordered_set<vertex_t>;
    using neighbors_data_t = std::vector<neighbors_list_t>;

 public:
    FastNeighborsGraph(size_t n_vertices, bool is_directed)
        : AdjListsGraph(n_vertices, is_directed), neighbors_data_(n_vertices) {}

    void add_edge(vertex_t from, vertex_t to) override {
        AdjListsGraph::add_edge(from, to);
        neighbors_data_[from].insert(to);
        if (!is_directed_)
            neighbors_data_[to].insert(from);
    }

    vertex_t add_vertex() override {
        auto new_vertex = AdjListsGraph::add_vertex();
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

}

using namespace graph;

class EncodedGraph : public FastNeighborsGraph
{
    struct dependency_t
    {
        bool sign;
        vertex_t offset;
    };

 public:
    using permutation_t = std::vector<vertex_t>;

    EncodedGraph(size_t n_vertices, bool is_directed)
        : FastNeighborsGraph(n_vertices, is_directed) {}

    void add_encoded_edge(vertex_t from, vertex_t to, vertex_t code) {
        FastNeighborsGraph::add_edge(from, to);
        edge_encodings_.push_back(code);
        if (!is_directed_)
            edge_encodings_.push_back(code);
    }

    permutation_t get_decode_permutation() {
        std::vector<dependency_t> deps(n_vertices_);
        permutation_t answer = permutation_t(n_vertices_);
        vertex_t init = random() % n_vertices_;

        vertex_t init_v = find_dependencies_or_answer_(init, deps);

        if (init_v != -1) {
            can_apply_(init, init_v, deps, answer);
            return answer;
        }

        vertex_t maximals[2] = {INT64_MIN, INT64_MIN};

        for (vertex_t v = 0; v < (vertex_t)n_vertices_; ++v) {
            auto& m = maximals[deps[v].sign];
            m = (deps[v].offset > m ? deps[v].offset : m);
        }

        for (int64_t i = 0; i < 2; ++i) {
            if (maximals[i] == INT64_MIN) continue;
            init_v = (i ? -1 : 1) * ((vertex_t)n_vertices_ - 1 - maximals[i]);
            if (can_apply_(init, init_v, deps, answer))
                break;
        }

        return answer;
    }

 private:
    std::vector<vertex_t> edge_encodings_;

    vertex_t find_dependencies_or_answer_dfs_(
        vertex_t v,
        vertex_t init,
        std::vector<bool>& visited,
        dependency_t dep,
        std::vector<dependency_t>& deps
    ) const {
        visited[v] = true;
        vertex_t answer = -1;
        for (auto e: outgoing_edges_by_reference(v)) {
            auto u = get_edge_end(e);
            if (u == v) continue;
            auto new_dep = dependency_t{!dep.sign, edge_encodings_[e] - dep.offset};
            if (!visited[u]) {
                deps[u] = new_dep;
                answer = find_dependencies_or_answer_dfs_(u, init, visited, new_dep, deps);
            }
            else if (new_dep.sign != deps[u].sign) {
                answer = std::abs(new_dep.offset - deps[u].offset) / 2;
            }
        }
        return answer;
    }

    vertex_t find_dependencies_or_answer_(
        vertex_t init,
        std::vector<dependency_t>& deps
    ) const {
        std::vector<bool> visited(n_vertices_);
        visited[init] = true;
        deps[init] = dependency_t{false, 0};
        return find_dependencies_or_answer_dfs_(init, init, visited, deps[init], deps);
    }

    bool can_apply_(
        vertex_t init,
        vertex_t value,
        std::vector<dependency_t>& deps,
        permutation_t& answer
    ) const {
        std::vector<bool> used(n_vertices_, false);
        used[value] = true;
        for (vertex_t v = 0; v < (vertex_t)n_vertices_; ++v) {
            if (v == init) continue;
            auto dep = deps[v];
            auto expect = (dep.sign ? -1 : 1) * value + dep.offset;
            if (expect >= (vertex_t)n_vertices_ || expect < 0 || used[expect])
                return false;
            answer[v] = expect;
            used[expect] = true;
        }
        answer[init] = value;
        return true;
    }
};

using std::cout;
using std::cin;

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    size_t n_vertices, n_edges;
    cin >> n_vertices >> n_edges;

    EncodedGraph g(n_vertices, false);
    for (size_t i = 0; i < n_edges; ++i) {
        vertex_t from, to, code;
        cin >> from >> to >> code;
        g.add_encoded_edge(from - 1, to - 1, code - 2);
    }

    if (n_vertices < 3) {
        for (size_t i = 1; i <= n_vertices; ++i)
            cout << i << ' ';
        return 0;
    }

    auto answer = g.get_decode_permutation();
    for (auto i: answer)
        cout << i + 1 << ' ';
}