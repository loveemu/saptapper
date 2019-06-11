/// @file
/// Primitive types and functions for GBA.

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <cstdint>

namespace saptapper {

using agbptr_t = std::uint32_t;

using agboff_t = std::int32_t;

using agbsize_t = std::uint32_t;

static constexpr bool is_romptr(agbptr_t addr) {
  return addr >= 0x8000000 && addr <= 0x9ffffff;
}

static constexpr agboff_t to_offset(agbptr_t addr) { return addr & 0x1ffffff; }

static constexpr agboff_t to_address(agboff_t offset) {
  return 0x8000000 | offset;
}

}  // namespace saptapper

#endif
