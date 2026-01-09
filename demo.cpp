// clang++ @wasm_flags.txt --output=build/demo.wasm demo.cpp
// #include <type_traits>

#pragma GCC poison new delete

// https://en.cppreference.com/w/cpp/language/types.html#Keywords
#pragma GCC poison void bool true false char char16_t char32_t wchar_t int short long float double

// static_assert(std::is_signed_v<signed>);
static_assert(sizeof(signed) == 4);
using i32 = signed;
#pragma GCC poison signed
static_assert(sizeof(i32 *) == sizeof(i32));

static_assert(sizeof(unsigned) == sizeof(i32));
using u32 = unsigned;
#pragma GCC poison unsigned
static_assert(sizeof(u32 *) == sizeof(i32));

i32 add(i32 a, i32 b) { return a + b; }
