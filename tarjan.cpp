// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <iostream>
#include <ranges>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

class vertex {
  static std::size_t component_count;
  static std::ptrdiff_t *chain, *index_iter;
  std::ptrdiff_t *low_link;
  const struct children {
    const children *next;
    struct vertex *vertex;
  } *children;

  [[nodiscard]] std::ptrdiff_t *stack_after(std::ptrdiff_t parent_index,
                                            const std::ptrdiff_t *parent_link) noexcept {
    *chain = chain - parent_link;
    low_link = chain++;
    *chain++ = parent_index;
    return low_link;
  }

public:
  [[nodiscard]] std::ptrdiff_t *stack_after() noexcept { return stack_after(-1, chain); }
  [[nodiscard]] bool visited() const noexcept { return low_link != nullptr; }
  void add_child(vertex &child) noexcept {
    static std::array<struct children, edge_count_max> edge_pool;
    static auto edge_pool_ptr = edge_pool.begin();

    *edge_pool_ptr = {.next = children, .vertex = &child};
    children = edge_pool_ptr++;
  }
  void find_component(std::ptrdiff_t *link) noexcept {
    while (children != nullptr) {
      const auto child = children->vertex;
      children = children->next;
      if (child->visited()) {
        low_link = std::min(low_link, child->low_link);
      } else {
        const auto child_link = child->stack_after(this - vertex_begin, link);
        __attribute__((musttail)) return child->find_component(child_link);
      }
    }
    const auto parent_link = link - *link;
    const auto parent_index = link[1];
    *link = this - vertex_begin;
    if (low_link == link) {
      ++component_count;
      const auto component_size = (chain - link) >> 1;
      while (chain != link) {
        chain -= 2;
        const auto i = *--index_iter = *chain;
        vertices[i].low_link = index_iter;
      }
      *--index_iter = component_size;
      if (parent_index == -1) {
        return;
      }
    } else {
      auto &parent = vertices[parent_index];
      parent.low_link = std::min(parent.low_link, low_link);
    }
    __attribute__((musttail)) return vertices[parent_index].find_component(parent_link);
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
std::ptrdiff_t *vertex::chain = indices.begin();
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
      vertex.find_component(vertex.stack_after());
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
