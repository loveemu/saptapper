// Saptapper: Automated GSF ripper for MusicPlayer2000.

#ifndef SAPTAPPER_TYPES_HPP_
#define SAPTAPPER_TYPES_HPP_

#include <cstdint>
#include <cassert>
#include <sstream>

namespace saptapper {

using agbptr_t = std::uint32_t;

using agbsize_t = std::uint32_t;

static constexpr auto agbnullptr{static_cast<agbptr_t>(-1)};

static constexpr auto agbnpos{static_cast<agbsize_t>(-1)};

static constexpr auto kAgbRomBase{static_cast<agbptr_t>(0x8000000)};

static constexpr auto kAgbEwramBase{static_cast<agbptr_t>(0x2000000)};

static constexpr auto kAgbRomEntrypoint{static_cast<agbsize_t>(0)};

static constexpr auto kAgbRamEntrypoint{static_cast<agbsize_t>(0xc0)};

static constexpr bool is_romptr(agbptr_t addr) {
  return addr >= 0x8000000 && addr <= 0x9ffffff;
}

static constexpr agbsize_t to_offset(agbptr_t addr) {
  return is_romptr(addr) ? addr & 0x1ffffff : addr & 0xffffff;
}

static constexpr agbptr_t remove_offset(agbptr_t addr) {
  return is_romptr(addr) ? addr & ~0x1ffffff : addr & ~0xffffff;
}

static constexpr bool is_romptr(agbptr_t addr, agbptr_t entrypoint) {
  return remove_offset(addr) == entrypoint;
}

static constexpr agbptr_t to_agbptr(agbptr_t base, agbsize_t offset) {
  return base + offset;
}

static std::string to_string(agbptr_t addr) {
  if (addr == agbnullptr) {
    return "null";
  } else {
    std::stringstream sstream;
    sstream << std::showbase << std::hex << addr;
    return sstream.str();
  }
}

}  // namespace saptapper

#endif
