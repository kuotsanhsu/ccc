// #pragma GCC optimize("Ofast")
#include <algorithm>
#include <forward_list>
#include <iostream>
#include <memory_resource>
#include <ranges>

struct Vertex {
private:
  static inline std::array<std::byte, 500'000 * (sizeof(nullptr) + sizeof(Vertex *))> buffer;
  static inline std::pmr::monotonic_buffer_resource mbr{buffer.data(), buffer.size()};
  static inline std::pmr::polymorphic_allocator<Vertex *> pa{&mbr};

public:
  Vertex **rindex{nullptr};
  std::pmr::forward_list<Vertex *> successors{pa};
  decltype(successors)::const_iterator last_successor{successors.cbefore_begin()};
};

void scc(const std::span<Vertex> vertices, const std::span<Vertex *> stack) {
  const Vertex *const front = &vertices.front();
  auto top = &*stack.begin();
  auto component = &*stack.end();
  size_t K = 0;

  const auto tarjan = [&top, &component, &K](this auto &&self, Vertex *const v) -> Vertex ** {
    if (v->rindex) {
      return v->rindex;
    }
    const auto rindex = v->rindex = top;
    *top++ = v;
    v->rindex = std::ranges::fold_left(
        v->successors | std::views::transform([self](auto &&v) { return self(v); }), rindex,
        [](auto &&a, auto &&b) { return std::min(a, b); });
    if (v->rindex == rindex) {
      std::for_each(rindex, top, [last = component, &component](Vertex *pw) -> void {
        *--component = pw;
        pw->rindex = last;
      });
      top = rindex;
      ++K;
    }
    return v->rindex;
  };

  for (auto &vertex : vertices) {
    tarjan(&vertex);
  }

  std::cout << K << '\n';
  for (auto v = component; v != &*stack.end();) {
    const auto last = (*v)->rindex;
    std::cout << last - v;
    while (v != last) {
      std::cout << ' ' << *v++ - front;
    }
    std::cout << '\n';
  }
}

int main() {
  std::cin.tie(nullptr)->sync_with_stdio(false);
  std::istream_iterator<size_t> ints(std::cin);
  const auto N = *ints++;
  const auto M = *ints++;
  static std::array<Vertex, 500'000> vertices;
  for (const auto _ : std::views::iota(size_t{}, M)) {
    auto &from = vertices[*ints++];
    auto &to = vertices[*ints++];
    from.last_successor = from.successors.insert_after(from.last_successor, &to);
  }
  static std::array<Vertex *, vertices.size()> stack;
  scc({std::begin(vertices), N}, stack);
}
