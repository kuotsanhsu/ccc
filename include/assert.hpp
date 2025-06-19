#pragma once
#include <cassert>

// clang-format off
#define consteval_assert(e) if consteval { assert(e); }
// clang-format on
