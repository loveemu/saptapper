/// @file
/// ARM/THUMB instruction utility.

#ifndef ARM_HPP_
#define ARM_HPP_

#include <cassert>
#include <cstdint>
#include "types.hpp"

namespace saptapper {

using armins_t = std::uint32_t;

using thumbins_t = std::uint16_t;

static constexpr bool is_arm_b(const armins_t ins) {
  return (ins & 0xff000000) == 0xea000000;
}

static constexpr armins_t make_arm_b(const agbptr_t current,
                                     const agbptr_t dest) {
  assert(current % 4 == 0 && dest % 4 == 0);
  const agbsize_t offset = dest - (current + 8);
  return 0xea000000 | ((offset / 4) & 0xffffff);
}

static constexpr agbptr_t arm_b_dest(const agbptr_t current,
                                     const armins_t ins) {
  agbsize_t offset = ins & 0xffffff;
  if ((offset & 0x800000) != 0) {
    offset |= ~0xffffff;
  }
  offset *= 4;

  return current + 8 + offset;
}

}  // namespace saptapper

#endif
