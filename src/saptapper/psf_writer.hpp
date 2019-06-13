#ifndef SAPTAPPER_PSF_WRITER_HPP_
#define SAPTAPPER_PSF_WRITER_HPP_

#include <cstdint>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include "zstr.hpp"

namespace saptapper {

class PsfWriter {
 public:
  PsfWriter(uint8_t version, std::map<std::string, std::string> tags = {});

  uint8_t version() const noexcept { return version_; }
  std::ostream& exe() noexcept { return exe_; }
  std::ostream& reserved() noexcept { return reserved_; }
  std::map<std::string, std::string>& tags() noexcept { return tags_; }

  void SaveToFile(const std::filesystem::path& path);
  void SaveToStream(std::ostream& out);

 private:
  uint8_t version_;
  std::ostringstream reserved_;
  std::ostringstream compressed_exe_;
  zstr::ostream exe_;
  std::map<std::string, std::string> tags_;

  std::string NewHeader(std::string_view compressed_exe,
                        std::string_view reserved,
                        std::uint32_t compressed_exe_crc32) const;
};

}  // namespace saptapper

#endif
