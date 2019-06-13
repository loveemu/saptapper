/// @file
/// File I/O functions.

#ifndef SAPTAPPER_FILE_IO_HPP_
#define SAPTAPPER_FILE_IO_HPP_

#include <filesystem>
#include <fstream>
#include <string_view>

namespace saptapper {

static void WriteAllBytesToFile(const std::filesystem::path& path,
                          std::string_view data) {
  std::ofstream stream(path, std::ios::out | std::ios::binary);
  stream.exceptions(std::ios::badbit | std::ios::eofbit | std::ios::failbit);
  stream.write(data.data(), data.size());
  stream.close();
}

}  // namespace saptapper

#endif
