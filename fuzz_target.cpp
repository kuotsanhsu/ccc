#include <cassert>
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
  if (size > 0 && data[0] == 'H') {
    if (size > 1 && data[1] == 'i') {
      if (size > 2 && data[2] == '!') {
        if (size == 3) {
          assert(false);
        }
      }
    }
  }
  return 0;
}
