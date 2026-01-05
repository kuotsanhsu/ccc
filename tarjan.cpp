// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <cstddef>
#include <iostream>
#include <optional>
#include <ranges>
#include <utility>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;

class vertex {
public:
  class full_vertex {
  public:
    class state {
      std::ptrdiff_t parent_link_offset;
      vertex *parent;

    public:
      state(full_vertex fv) noexcept : parent_link_offset(chain - fv.link), parent(fv.vertex) {}
      [[nodiscard]] full_vertex destruct(vertex *vertex) noexcept {
        return {std::exchange(parent, vertex), this - parent_link_offset};
      }
      /// `collect` must be called after `destruct`.
      [[nodiscard]] vertex *collect() const noexcept {
        parent->low_link = max_link;
        return parent;
      }
    };

    full_vertex() noexcept : vertex(nullptr), link(chain) {}

    [[nodiscard]] full_vertex push(struct vertex *child) const noexcept {
      *chain = state(*this);
      return {child, child->low_link = chain++};
    }

    [[nodiscard]] std::optional<full_vertex> child() const noexcept {
      while (vertex->children != nullptr) {
        const auto child = vertex->children->vertex;
        vertex->children = vertex->children->next;
        if (child->visited()) {
          vertex->low_link = std::min(vertex->low_link, child->low_link);
        } else {
          return push(child);
        }
      }
      return std::nullopt;
    }
    void update_low_link(const full_vertex &other) const noexcept {
      vertex->low_link = std::min(vertex->low_link, other.vertex->low_link);
    }
    [[nodiscard]] bool done() const noexcept { return vertex->low_link == link; }

    [[nodiscard]] full_vertex parent() const noexcept {
      const auto parent = link->destruct(vertex);
      return parent;
    }

    [[nodiscard]] std::ranges::sized_range auto component() const noexcept {
      return std::ranges::subrange(link, std::exchange(chain, link)) |
             std::views::transform(&state::collect) | std::views::reverse;
    }

  private:
    static state *chain;
    vertex *vertex;
    state *link;
    full_vertex(struct vertex *vertex, state *link) noexcept : vertex(vertex), link(link) {}
  };

  [[nodiscard]] static auto find_component(const full_vertex &curr) noexcept {
    // if (auto child = curr.child()) {
    //   __attribute__((musttail)) return find_component(*child);
    // }
    const auto parent = curr.parent();
    if (curr.done()) {
      return curr.component();
    }
    parent.update_low_link(curr);
    __attribute__((musttail)) return find_component(parent);
  }

  [[nodiscard]] bool visited() const noexcept { return low_link != nullptr; }
  void add_child(vertex &child) noexcept {
    static std::array<struct children, edge_count_max> edge_pool;
    static auto edge_pool_ptr = edge_pool.begin();

    *edge_pool_ptr = {.next = children, .vertex = &child};
    children = edge_pool_ptr++;
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
  full_vertex::state *low_link;
  const struct children {
    const children *next;
    struct vertex *vertex;
  } *children;

  static constinit std::array<vertex, vertex_count_max> vertices;
  static constinit const vertex *const vertex_begin;
  static constinit std::array<std::ptrdiff_t, vertex_count_max << 1> indices;
  static full_vertex::state *const max_link;
  static std::ptrdiff_t *index_iter;
  static std::size_t component_count;
};

constinit std::array<vertex, vertex_count_max> vertex::vertices{};
constinit const vertex *const vertex::vertex_begin = vertices.cbegin();

static_assert(sizeof(std::ptrdiff_t) == sizeof(vertex *));
static_assert(sizeof(vertex::full_vertex::state) == sizeof(std::ptrdiff_t) * 2);
static_assert(alignof(vertex::full_vertex::state) == alignof(std::ptrdiff_t));

constinit std::array<std::ptrdiff_t, vertex_count_max << 1> vertex::indices{};
vertex::full_vertex::state *vertex::full_vertex::chain =
    reinterpret_cast<vertex::full_vertex::state *>(indices.begin());
vertex::full_vertex::state *const vertex::max_link =
    reinterpret_cast<vertex::full_vertex::state *>(indices.end());
std::ptrdiff_t *vertex::index_iter = indices.end();
std::size_t vertex::component_count = 0;

namespace {
void preallocated(std::size_t vertex_count, std::ranges::input_range auto &&edges) {
  auto vertices = vertex::get_vertices(vertex_count);
  for (const auto [source, target] : edges) {
    vertices[source].add_child(vertices[target]);
  }
  for (auto &vertex : vertices) {
    if (!vertex.visited()) {
      const auto component = vertex::find_component(vertex::full_vertex().push(&vertex));
      const auto component_size = std::ranges::size(component);
      for (const auto v : component) {
        v - nullptr;
      }
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
