#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <type_traits>

template <std::integral T> class vector {
public:
  using value_type = T;
  static_assert(std::is_same_v<value_type, T>);

  // using allocator_type = Allocator;

  using size_type = std::size_t;
  static_assert(std::unsigned_integral<size_type>);

  using difference_type = std::ptrdiff_t;
  static_assert(std::signed_integral<difference_type>);

  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;

  using iterator = pointer;
  static_assert(std::contiguous_iterator<iterator>);
  static_assert(std::is_same_v<std::iter_value_t<iterator>, value_type>);

  using const_iterator = const_pointer;
  static_assert(std::contiguous_iterator<const_iterator>);
  static_assert(std::is_same_v<std::iter_value_t<const_iterator>, value_type>);

  using reverse_iterator = std::reverse_iterator<iterator>;
  static_assert(std::random_access_iterator<reverse_iterator>);
  static_assert(!std::contiguous_iterator<reverse_iterator>);
  static_assert(std::is_same_v<std::iter_value_t<reverse_iterator>, value_type>);

  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  static_assert(std::random_access_iterator<const_reverse_iterator>);
  static_assert(!std::contiguous_iterator<const_reverse_iterator>);
  static_assert(std::is_same_v<std::iter_value_t<const_reverse_iterator>, value_type>);

  constexpr vector() noexcept = default;
  constexpr ~vector() {
    /** https://man7.org/linux/man-pages/man3/free.3p.html
     *
     * The free() function shall cause the space pointed to by ptr to be deallocated; that is, made
     * available for further allocation. If ptr is a null pointer, no action shall occur. Otherwise,
     * if the argument does not match a pointer earlier returned by a function in POSIX.1‐2008 that
     * allocates memory as if by malloc(), or if the space has been deallocated by a call to free()
     * or realloc(), the behavior is undefined.
     *
     * Any use of a pointer that refers to freed space results in undefined behavior.
     */
    // std::free(buffer);

    /** https://www.reddit.com/r/cpp_questions/comments/no536z/comment/gzy9f23/
     *
     * It's safe to call delete on null pointers per 6.7.5.4.2.4 in the C++ standard.
     *
     * https://en.cppreference.com/w/cpp/language/delete.html
     *
     * If ptr is a null pointer value, no destructors are called, and the deallocation function may
     * or may not be called (it's unspecified), but the default deallocation functions are
     * guaranteed to do nothing when passed a null pointer.
     *
     * https://stackoverflow.com/a/17344873
     *
     * If T has a destructor then it will be invoked by delete[]. From section 5.3.5 Delete of the
     * c++11 standard (draft n3337), clause 6:
     */
    delete[] buffer;
    buffer = nullptr;
    m_capacity = 0;
    m_size = 0;
  }

  [[nodiscard]] constexpr reference at(size_type pos) {
    if (pos >= size()) {
      throw std::out_of_range("vector access out of range");
    }
    return buffer[pos];
  }
  [[nodiscard]] constexpr const_reference at(size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("vector access out of range");
    }
    return buffer[pos];
  }

  [[nodiscard]] constexpr reference operator[](size_type pos) { return buffer[pos]; }
  [[nodiscard]] constexpr const_reference operator[](size_type pos) const { return buffer[pos]; }

  [[nodiscard]] constexpr reference front() { return *begin(); }
  [[nodiscard]] constexpr const_reference front() const { return *begin(); }

  [[nodiscard]] constexpr reference back() { return *std::prev(end()); }
  [[nodiscard]] constexpr const_reference back() const { return *std::prev(end()); }

  [[nodiscard]] constexpr T *data() noexcept { return buffer; }
  [[nodiscard]] constexpr const T *data() const noexcept { return buffer; }

  [[nodiscard]] constexpr iterator begin() const noexcept { return buffer; }
  [[nodiscard]] constexpr iterator end() const noexcept { return buffer + m_size; }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max();
  }
  constexpr void reserve(size_type new_cap) {
    if (new_cap > max_size()) {
      throw std::length_error("vector reserve out of space");
    }
    if (new_cap > capacity()) {
      auto new_buffer = new value_type[new_cap];
      std::ranges::copy(*this, new_buffer);
      const auto old_size = size();
      ~vector(); // Sets capacity and size to 0.
      buffer = new_buffer;
      m_capacity = new_cap;
      m_size = old_size;
    }
  }
  [[nodiscard]] constexpr size_type capacity() const noexcept { return m_capacity; }

  constexpr void clear() {
    /** https://stackoverflow.com/a/456339
     *
     * The notation for explicit call of a destructor can be used for any scalar type name.
     * Allowing this makes it possible to write code without having to know if a destructor exists
     * for a given type.
     */
    for (auto &t : *this | std::views::reverse) {
      t.~T();
    }
  }

  constexpr void push_back(const_reference value) {}

private:
  pointer buffer = nullptr;
  size_type m_capacity = 0;
  size_type m_size = 0;
};

using vec = vector<int>;

static_assert(std::is_same_v<vec::value_type, int>);

static_assert(std::unsigned_integral<vec::size_type>);
static_assert(std::is_unsigned_v<vec::size_type>);
static_assert(std::is_same_v<vec::size_type, std::size_t>);

static_assert(std::signed_integral<vec::difference_type>);
static_assert(std::is_signed_v<vec::difference_type>);
static_assert(std::is_same_v<std::iter_difference_t<vec>, std::ptrdiff_t>);

static_assert(std::contiguous_iterator<vec::iterator>);
static_assert(std::is_same_v<std::iter_value_t<vec::iterator>, int>);

static_assert(std::ranges::sized_range<vec>);

static constexpr vec v;
static_assert(v.empty());
static_assert([] {
  vec v;
  v.clear();
  return true;
}());
