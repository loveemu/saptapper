/// @file
/// Header file for GBA Cartridge class.

#ifndef CARTRIDGE_HPP_
#define CARTRIDGE_HPP_

#include <filesystem>
#include <string>
#include "types.hpp"

namespace saptapper {

class Cartridge {
 public:
  using size_type = std::string::size_type;

  static constexpr agbsize_t kHeaderSize = 0x100;
  static constexpr agbsize_t kMaximumSize = 0x2000000;

  Cartridge() = default;

  std::string& data() { return data_; }
  const std::string& data() const { return data_; }
  size_type size() const { return data_.size(); }
  std::string game_title() const { return data_.substr(0xa0, 12); }
  std::string game_code() const { return data_.substr(0xac, 4); }

  static Cartridge LoadFromFile(const std::filesystem::path& path);

 private:
  std::string data_;

  static void ValidateSize(size_type size);
};

}  // namespace saptapper

#endif
