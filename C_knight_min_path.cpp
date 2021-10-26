#include <iostream>
#include <vector>
#include <set>
#include <queue>

namespace graph
{

using vertex_t = int32_t;
using distance_t = int32_t;
using adj_list_t = std::vector<vertex_t>;

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

class Graph
{
 public:
    virtual const adj_list_t& get_neighbors(vertex_t v) const = 0;
    virtual void add_edge(vertex_t from, vertex_t to) = 0;

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
        adj_lists_[from].push_back(to);
        if (!is_directed_)
            adj_lists_[to].push_back(from);
    }

 private:
    adj_lists_t adj_lists_;
};

namespace impl
{

struct path_data_t
{
    std::vector<distance_t> dist;
    std::vector<vertex_t> prev;
};

static constexpr distance_t INF_DISTANCE = -1;

decltype(auto) find_shortest_paths_from_vertex(const Graph& g, vertex_t s) {
    std::vector<distance_t> dist(g.n_vertices() + 1, INF_DISTANCE);
    std::vector<vertex_t> prev(g.n_vertices() + 1);

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

decltype(auto) find_shortest_path(const Graph& g, vertex_t from, vertex_t to) {
    auto[dist, prev] = impl::find_shortest_paths_from_vertex(g, from);
    distance_t distance = dist[to];
    std::vector<vertex_t> path(distance + 1);
    vertex_t curr = to;
    for (int i = distance; i >= 0; --i) {
        path[i] = curr;
        curr = prev[curr];
    }
    return path;
}

}

using namespace graph;

using std::cout;
using std::cin;

template<class T>
class Grid
{
 public:
    using value_t = T;

    Grid(int n_rows, int n_cols)
        : encoder_(n_cols, true), n_cols_(n_cols), n_rows_(n_rows) {}

    bool contains_cell(int x, int y) const {
        return (x >= 0 && x < n_cols_) && (y >= 0 && y < n_rows_);
    }

    decltype(auto) get_encoder() const {
        return encoder_;
    }

    decltype(auto) shape() const {
        return std::make_pair(n_rows_, n_cols_);
    }

    int n_cells() const {
        return n_rows_ * n_cols_;
    }

 private:
    PairEncoder<value_t> encoder_;
    int n_cols_;
    int n_rows_;
};

AdjListsGraph generate_knight_moves_graph(const Grid<vertex_t>& grid) {
    AdjListsGraph moves_graph(grid.n_cells(), false);
    const auto& encoder = grid.get_encoder();
    auto [n_rows, n_cols] = grid.shape();

    std::pair<int, int> knight_moves[8] = {
        {1,  -2},
        {1,  2},
        {-1, -2},
        {-1, 2},
        {2,  -1},
        {2,  1},
        {-2, -1},
        {-2, 1}
    };

    for (int x = 0; x < n_cols; ++x) {
        for (int y = 0; y < n_rows; ++y) {
            int code = encoder.encode(x, y);
            for (auto m: knight_moves) {
                auto[new_x, new_y] = std::make_pair(x + m.first, y + m.second);
                if (grid.contains_cell(new_x, new_y)) {
                    moves_graph.add_edge(code, encoder.encode(new_x, new_y));
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
    auto grid = Grid<vertex_t>(chess_board_size, chess_board_size);
    auto g = generate_knight_moves_graph(grid);


    int start_x, start_y, finish_x, finish_y;
    cin >> start_x >> start_y >> finish_x >> finish_y;
    auto[start, finish] = std::make_pair(
        grid.get_encoder().encode(start_x - 1, start_y - 1),
        grid.get_encoder().encode(finish_x - 1, finish_y - 1)
    );
    auto knight_path = find_shortest_path(g, start, finish);
    cout << knight_path.size() - 1 << '\n';
    for (auto pos_code: knight_path) {
        auto[x, y] = grid.get_encoder().decode(pos_code);
        cout << x + 1 << ' ' << y + 1 << '\n';
    }
}