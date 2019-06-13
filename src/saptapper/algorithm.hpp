/// @file
/// Algorithms.

#ifndef SAPTAPPER_ALGORITHM_HPP_
#define SAPTAPPER_ALGORITHM_HPP_

#include <array>
#include <cassert>
#include <cstring>
#include <string_view>
#include "types.hpp"

namespace saptapper {

static bool memcmp_loose(const char* buf1, const char* buf2, size_t n,
                         unsigned int max_diff) {
  unsigned int diff = 0;
  for (size_t pos = 0; pos < n; pos++) {
    if (buf1[pos] != buf2[pos]) {
      if (++diff >= max_diff) return false;
    }
  }
  return true;
}

static agbptr_t find_loose(std::string_view rom, std::string_view pattern,
                           unsigned int max_diff, agbsize_t pos = 0) {
  if (rom.size() < pattern.size()) return agbnullptr;

  constexpr agbsize_t align = 4;
  for (agbsize_t offset = pos; offset < rom.size() - pattern.size();
       offset += align) {
    if (memcmp_loose(rom.data() + offset, pattern.data(), pattern.size(),
                     max_diff))
      return to_romptr(offset);
  }
  return agbnullptr;
}

template <size_t _Size>
static agbptr_t find_backwards(std::string_view rom,
                               std::array<std::string_view, _Size> patterns,
                               agbsize_t pos, agbsize_t length) {
  if (pos >= rom.size()) return agbnullptr;

  constexpr agbsize_t align = 4;
  assert(length % align == 0);
  if (length < align || rom.size() < length) return agbnullptr;

  const agbsize_t max_pos = pos - align;
  const agbsize_t min_pos = pos - length;
  for (agbsize_t offset = max_pos; offset >= min_pos; offset -= align) {
    for (const auto& pattern : patterns) {
      if (std::memcmp(rom.data() + offset, pattern.data(), pattern.size()) ==
          0) {
        return to_romptr(offset);
      }
    }
  }
  return agbnullptr;
}

}  // namespace saptapper

#endif
