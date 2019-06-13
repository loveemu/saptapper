/// @file
/// Header file for GBA Cartridge class.

#ifndef CARTRIDGE_HPP_
#define CARTRIDGE_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include "types.hpp"

namespace saptapper {

class Cartridge {
 public:
  using size_type = agbsize_t;

  static constexpr agbsize_t kHeaderSize = 0x100;
  static constexpr agbsize_t kMaximumSize = 0x2000000;

  Cartridge() = default;

  std::string& rom() { return rom_; }
  const std::string& rom() const { return rom_; }
  size_type size() const { return static_cast<agbsize_t>(rom_.size()); }
  std::string game_title() const { return rom_.substr(0xa0, 12); }
  std::string game_code() const { return rom_.substr(0xac, 4); }

  static Cartridge LoadFromFile(const std::filesystem::path& path);

 private:
  std::string rom_;

  static void ValidateSize(std::uintmax_t size);
};

}  // namespace saptapper

#endif
