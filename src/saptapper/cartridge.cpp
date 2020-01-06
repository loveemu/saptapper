// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "cartridge.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace saptapper {

Cartridge Cartridge::LoadFromFile(const std::filesystem::path& path) {
  const std::string ext{path.extension().string()};
  const agbptr_t entrypoint =
      (ext == ".mb" || ext == ".MB") ? kAgbEwramBase : kAgbRomBase;
  return LoadFromFile(path, entrypoint);
}

Cartridge Cartridge::LoadFromFile(const std::filesystem::path& path, agbptr_t entrypoint) {
  Cartridge cartridge;

  if (entrypoint != kAgbRomBase && entrypoint != kAgbEwramBase)
    throw std::range_error("Unexpected entrypoint address.");

  const auto size = file_size(path);
  ValidateSize(size);

  std::ifstream stream(path, std::ios::in | std::ios::binary);
  stream.exceptions(std::ios::badbit | std::ios::eofbit | std::ios::failbit);

  const auto aligned_size = (size + 3) & ~3;
  std::string rom(aligned_size, 0);
  stream.read(rom.data(), size);
  stream.close();

  cartridge.rom_ = std::move(rom);
  cartridge.entrypoint_ = entrypoint;
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
