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

int encode_pair(int x, int y, int key) {
    return (x * key + y);
}

decltype(auto) decode_pair(int pair_code, int key) {
    return std::make_pair(pair_code / key, pair_code % key);
}

bool is_correct(int x, int y, int grid_size) {
    return (x >= 0 && x < grid_size) && (y >= 0 && y < grid_size);
}

AdjListsGraph generate_knight_moves_graph(int grid_size) {
    AdjListsGraph moves_graph(grid_size * grid_size, false);

    std::pair<int, int> knight_moves[8] = {
        {1, -2}, {1, 2}, {-1, -2}, {-1, 2},
        {2, -1}, {2, 1}, {-2, -1}, {-2, 1}
    };

    for (int x = 0; x < grid_size; ++x) {
        for (int y = 0; y < grid_size; ++y) {
            int code = encode_pair(x, y, grid_size);
            for (auto m : knight_moves) {
                auto[new_x, new_y] = std::make_pair(x + m.first, y + m.second);
                if (is_correct(new_x, new_y, grid_size)) {
                    moves_graph.add_edge(code, encode_pair(new_x, new_y, grid_size));
                }
            }
        }
    }

    return moves_graph;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int chess_board_size;
    cin >> chess_board_size;
    int start_x, start_y, finish_x, finish_y;
    cin >> start_x >> start_y >> finish_x >> finish_y;
    auto[start, finish] = std::make_pair(
        encode_pair(start_x - 1, start_y - 1, chess_board_size),
        encode_pair(finish_x - 1, finish_y - 1, chess_board_size)
    );

    auto g = generate_knight_moves_graph(chess_board_size);
    auto knight_path = find_shortest_path(g, start, finish);
    cout << knight_path.size() - 1 << '\n';
    for (auto pos_code : knight_path) {
        auto[x, y] = decode_pair(pos_code, chess_board_size);
        cout << x + 1 << ' ' << y + 1 << '\n';
    }
}