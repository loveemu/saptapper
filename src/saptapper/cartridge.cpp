// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "cartridge.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace saptapper {

Cartridge Cartridge::LoadFromFile(const std::filesystem::path& path) {
  Cartridge cartridge;

  const auto size = file_size(path);
  ValidateSize(size);

  std::ifstream stream(path, std::ios::in | std::ios::binary);
  stream.exceptions(std::ios::badbit | std::ios::eofbit | std::ios::failbit);

  const auto aligned_size = (size + 3) & ~3;
  std::string rom(aligned_size, 0);
  stream.read(rom.data(), size);
  stream.close();

  cartridge.rom_ = std::move(rom);
  return cartridge;
}

void Cartridge::ValidateSize(std::uintmax_t size) {
  if (size < kHeaderSize) {
    throw std::range_error("The input data too small.");
  } else if (size > kMaximumSize) {
    throw std::range_error("The input data too large.");
  }
}

}  // namespace saptapper
