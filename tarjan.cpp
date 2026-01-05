// https://judge.yosupo.jp/problem/scc
// https://judge.yosupo.jp/submission/287255
// https://honam0905.github.io/CP-library
#include <cstddef>
#include <forward_list>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

// log2(500'000) is about 18.897
constexpr int vertex_count_max = 500'000;
constexpr int edge_count_max = 500'000;
class graph {
  class vertex {
    std::ptrdiff_t *low_link = nullptr;
    std::forward_list<std::reference_wrapper<vertex>> children;

  public:
    [[nodiscard]] std::ptrdiff_t index() const noexcept { return this - vertex_begin; }
    [[nodiscard]] bool equal_link(const std::ptrdiff_t *link) const noexcept {
      return low_link == link;
    }
    [[nodiscard]] bool visited() const noexcept { return low_link != nullptr; }
    void add_child(const vertex &child) noexcept { children.emplace_front(child); }
    void update_low_link(const vertex &other) noexcept {
      low_link = std::min(low_link, other.low_link);
    }
    vertex *consume_child() noexcept {
      while (!children.empty()) {
        vertex &child = children.front();
        children.pop_front();
        if (child.visited()) {
          update_low_link(child);
        } else {
          return &child;
        }
      }
      return nullptr;
    }
  };

public:
  class strong_component_iterator {
    class vertex_ptr {
    public:
      using pointer = vertex *;

    private:
      static pointer vertices;
      static std::ptrdiff_t *chain;
      std::ptrdiff_t index, *link;
      vertex_ptr(std::ptrdiff_t index, std::ptrdiff_t *link) noexcept : index(index), link(link) {}
      [[nodiscard]] pointer get() const noexcept { return vertices + index; }
      [[nodiscard]] vertex_ptr stack(vertex &vertex) const noexcept {
        *chain = chain - link;
        const auto low_link = chain++;
        vertex.low_link = low_link;
        *chain++ = index;
        return {&vertex - vertices, low_link};
      }

    public:
      vertex_ptr(vertex &vertex) noexcept : vertex_ptr(vertex_ptr(-1, nullptr).stack(vertex)) {}
      pointer operator->() const noexcept { return get(); }
      const vertex &operator*() const { return *get(); }
      explicit operator bool() const noexcept { return index != -1; }
      [[nodiscard]] vertex_ptr parent() noexcept {
        return {link[1], link - std::exchange(*link, index)};
      }
      [[nodiscard]] std::optional<vertex_ptr> child() const noexcept {
        if (const auto child = get()->consume_child()) {
          return stack(*child);
        }
        return std::nullopt;
      }
      [[nodiscard]] bool equal_link() const noexcept { return get()->equal_link(link); }
      [[nodiscard]] std::ptrdiff_t *shrink(std::ptrdiff_t *index_iter) noexcept {
        // const auto component_size = (chain - link) >> 1;
        while (chain != link) {
          chain -= 2;
          const auto i = *--index_iter = *chain;
          vertices[i].low_link = index_iter;
        }
        return index_iter;
      }
    } curr;
    std::ptrdiff_t *index_iter;

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = void;
    bool operator==(std::default_sentinel_t /*unused*/) const noexcept { return bool(curr); }
    strong_component_iterator(vertex &root) : curr(root) {}
    strong_component_iterator &operator++() noexcept {
      curr = find_component(curr);
      return *this;
    }

  private:
    vertex_ptr find_component(vertex_ptr curr) noexcept {
      if (auto child = curr.child()) {
        __attribute__((musttail)) return find_component(*child);
      }
      const auto parent = curr.parent();
      if (curr.equal_link()) {
        const auto old_index_iter = index_iter;
        index_iter = curr.shrink(index_iter);
        *--index_iter = old_index_iter - index_iter; // component_size
        return parent;
      }
      parent->update_low_link(*curr);
      __attribute__((musttail)) return find_component(parent);
    }
  };

  [[nodiscard]] std::size_t strong_component_begin() {
    std::size_t component_count = 0;
    for (auto &vertex : vertices) {
      if (!vertex.visited()) {
        for (strong_component_iterator sci(vertex); sci; ++sci) {
          ++component_count;
        }
      }
    }
  }

  graph(std::size_t vertex_count, std::ranges::input_range auto &&edges,
        const std::pmr::polymorphic_allocator<vertex> &alloc)
      : vertices(vertex_count, alloc) {
    for (const auto [source, target] : edges) {
      vertices[source].add_child(vertices[target]);
    }
  }

private:
  static constinit std::array<std::ptrdiff_t, vertex_count_max << 1> indices;
  std::pmr::vector<vertex> vertices;
};

constinit std::array<std::ptrdiff_t, vertex_count_max << 1> graph::indices{};

namespace std {
template <typename A, typename B> auto &operator>>(std::istream &cis, std::pair<A, B> &pair) {
  return cis >> pair.first >> pair.second;
}
} // namespace std

int main() {
  std::pmr::monotonic_buffer_resource vertex_pool(vertex_count_max * sizeof(graph::vertex),
                                                  std::pmr::null_memory_resource());

  std::cin.tie(nullptr);
  std::ios_base::sync_with_stdio(false);
  std::istream_iterator<std::pair<int, int>> pairs(std::cin);
  const auto [vertex_count, edge_count] = *pairs++;
  graph graph(vertex_count, std::views::counted(pairs, edge_count), &vertex_pool);

  vertex::print(std::cout);
}
