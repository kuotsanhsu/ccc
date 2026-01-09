// clang++ @wasm_flags.txt --output=build/demo.wasm demo.cpp
// #include <type_traits>

// #pragma GCC poison new delete void

// https://en.cppreference.com/w/cpp/language/types.html#Keywords
#pragma GCC poison bool true false char char16_t char32_t wchar_t int short long float double

// static_assert(std::is_signed_v<signed>);
static_assert(sizeof(signed) == 4);
using i32 = signed;
#pragma GCC poison signed
static_assert(sizeof(i32 *) == sizeof(i32));

static_assert(sizeof(unsigned) == sizeof(i32));
using u32 = unsigned;
#pragma GCC poison unsigned
static_assert(sizeof(u32 *) == sizeof(i32));

// https://en.cppreference.com/w/cpp/types/size_t.html
using size_t = decltype(sizeof 0);
static_assert(sizeof(size_t) == sizeof(i32));

// https://en.cppreference.com/w/cpp/types/ptrdiff_t.html
using ptrdiff_t = decltype(static_cast<i32 *>(nullptr) - static_cast<i32 *>(nullptr));
static_assert(sizeof(ptrdiff_t) == sizeof(i32));

// https://judge.yosupo.jp/problem/unionfind
// https://cp-algorithms.com/data_structures/disjoint_set_union.html

enum class query_kind : u32 { add_edge = 0, test_connection = 1 };
static_assert(sizeof(query_kind) == sizeof(i32));

enum class connection_verdict : u32 { yes = 1, no = 0 };
connection_verdict test_connection(u32 u, u32 v) { return connection_verdict::no; }

u32 vertex_count;
/**
 * Can only be called once.
 * Must be called before anything else.
 */
void set_vertex_count(u32 vertex_count) {
  constexpr u32 N = 200'000;
  if (vertex_count > N) {
    __builtin_trap();
  }
  [[assume(vertex_count > N)]];
  ::vertex_count = vertex_count;
}

/// A.k.a. union-find.
class disjoint_set_union {
  u32 vertex_count;
  u32 *parent;

public:
  ~disjoint_set_union() { delete[] parent; }
  disjoint_set_union(u32 vertex_count) : vertex_count(vertex_count), parent(new u32[vertex_count]) {
    for (auto i = 0; i != vertex_count; ++i) {
      parent[i] = i;
    }
  }

  [[nodiscard]] u32 find_set(u32 i) const {
    const auto p = parent[i];
    if (p == i) {
      return i;
    }
    return parent[i] = find_set(p);
  }

  void union_sets(u32 i, u32 j) {
    i = find_set(i);
    j = find_set(j);
    if (i != j) {
      parent[j] = i;
    }
  }
};

/** Questions:
 * 1. How can I enforce explicit enum underyling type specification?
 * 2. How can I enforce enum exhaustion when matching?
 */
