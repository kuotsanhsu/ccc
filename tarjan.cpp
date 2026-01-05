// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <iostream>
#include <ranges>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

class vertex {
  struct state;
  using link = state *;
  struct state {
    std::ptrdiff_t parent_link_offset;
    vertex *parent;
    [[nodiscard]] link parent_link() noexcept { return this - parent_link_offset; }
  };
  static_assert(sizeof(std::ptrdiff_t) == sizeof(vertex *));
  static_assert(sizeof(state) == sizeof(std::ptrdiff_t) * 2);
  static_assert(alignof(state) == alignof(std::ptrdiff_t));

  static std::size_t component_count;
  static link chain;
  static const link max_link;
  static std::ptrdiff_t *index_iter;
  link low_link;
  const struct children {
    const children *next;
    struct vertex *vertex;
  } *children;

  [[nodiscard]] link stack_after(vertex *parent, link parent_link) noexcept {
    *chain = {.parent_link_offset = chain - parent_link, .parent = parent};
    return low_link = chain++;
  }

public:
  [[nodiscard]] link stack_after() noexcept { return stack_after(nullptr, chain); }
  [[nodiscard]] bool visited() const noexcept { return low_link != nullptr; }
  void add_child(vertex &child) noexcept {
    static std::array<struct children, edge_count_max> edge_pool;
    static auto edge_pool_ptr = edge_pool.begin();

    *edge_pool_ptr = {.next = children, .vertex = &child};
    children = edge_pool_ptr++;
  }
  friend void find_component(vertex *curr, link curr_link) noexcept {
    while (curr->children != nullptr) {
      const auto child = curr->children->vertex;
      curr->children = curr->children->next;
      if (child->visited()) {
        curr->low_link = std::min(curr->low_link, child->low_link);
      } else {
        const auto child_link = child->stack_after(curr, curr_link);
        __attribute__((musttail)) return find_component(child, child_link);
      }
    }
    const auto parent = curr_link->parent;
    const auto parent_link = curr_link->parent_link();
    curr_link->parent_link_offset = curr - vertex_begin;
    if (curr->low_link == curr_link) {
      ++component_count;
      const auto component_size = chain - curr_link;
      while (chain != curr_link) {
        --chain;
        const auto i = *--index_iter = chain->parent_link_offset;
        vertices[i].low_link = max_link;
      }
      *--index_iter = component_size;
      if (parent == nullptr) {
        return;
      }
    } else {
      parent->low_link = std::min(parent->low_link, curr->low_link);
    }
    __attribute__((musttail)) return find_component(parent, parent_link);
  }

  static void print(std::ostream &cos) {
    cos << component_count << '\n';
    while (component_count--) {
      auto count = *index_iter++;
      cos << count;
      while (count--) {
        cos << ' ' << *index_iter++;
      }
      cos << '\n';
    }
  }

  static std::span<vertex> get_vertices(std::size_t count) { return {vertices.begin(), count}; }

private:
  static constinit std::array<std::ptrdiff_t, vertex_count_max << 1> indices;
  static constinit std::array<vertex, vertex_count_max> vertices;
  static constinit const vertex *const vertex_begin;
};

constinit std::array<std::ptrdiff_t, vertex_count_max << 1> vertex::indices{};
std::size_t vertex::component_count = 0;
vertex::link vertex::chain = reinterpret_cast<link>(indices.begin());
const vertex::link vertex::max_link = reinterpret_cast<link>(indices.end());
std::ptrdiff_t *vertex::index_iter = indices.end();
constinit std::array<vertex, vertex_count_max> vertex::vertices{};
constinit const vertex *const vertex::vertex_begin = vertices.cbegin();

namespace {
void preallocated(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  auto vertices = vertex::get_vertices(vertex_count);
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }
  for (auto &vertex : vertices) {
    if (!vertex.visited()) {
      find_component(&vertex, vertex.stack_after());
    }
  }
  vertex::print(std::cout);
}
} // namespace

namespace std {
template <typename A, typename B> auto &operator>>(std::istream &cis, std::pair<A, B> &pair) {
  return cis >> pair.first >> pair.second;
}
} // namespace std

int main() {
  std::cin.tie(nullptr);
  std::ios_base::sync_with_stdio(false);
  std::istream_iterator<std::pair<int, int>> pairs(std::cin);
  const auto [vertex_count, edge_count] = *pairs++;
  preallocated(vertex_count, std::views::counted(pairs, edge_count));
}
