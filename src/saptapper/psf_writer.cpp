// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "psf_writer.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include "bytes.hpp"

namespace saptapper {

PsfWriter::PsfWriter(uint8_t version, std::map<std::string, std::string> tags)
    : version_{version},
      exe_{compressed_exe_, zstr::default_buff_size, Z_BEST_COMPRESSION, 15},
      tags_(std::move(tags)) {}

void PsfWriter::SaveToFile(const std::filesystem::path& path,
                           const std::map<std::string, std::string>& tags) {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  file.exceptions(std::ios::badbit);
  SaveToStream(file, tags);
  file.close();
}

void PsfWriter::SaveToStream(std::ostream& out,
                             const std::map<std::string, std::string>& tags) {
  exe_.flush();
  reserved_.flush();

  const std::string compressed_exe = compressed_exe_.str();
  const std::string reserved = reserved_.str();
  const std::uint32_t compressed_exe_crc32 =
      crc32(0L, reinterpret_cast<const Bytef*>(compressed_exe.data()),
            static_cast<uInt>(compressed_exe.size()));

  const std::string header{
      NewHeader(compressed_exe, reserved, compressed_exe_crc32)};
  out.write(header.data(), header.size());
  out.write(reserved.data(), reserved.size());
  out.write(compressed_exe.data(), compressed_exe.size());

  if (!tags.empty()) {
    out.write("[TAG]", 5);

    for (const auto& tag : tags) {
      const auto& key = tag.first;
      const auto& value = tag.second;

      std::istringstream value_reader{value};
      std::string line;
      while (std::getline(value_reader, line))
        out << key << '=' << value << '\n';
    }
  }
}

std::string PsfWriter::NewHeader(std::string_view compressed_exe,
                                 std::string_view reserved,
                                 std::uint32_t compressed_exe_crc32) const {
  std::string header(16, 0);
  std::memcpy(header.data(), "PSF", 3);
  WriteInt8(&header[3], version_);
  WriteInt32L(&header[4], static_cast<std::uint32_t>(reserved.size()));
  WriteInt32L(&header[8], static_cast<std::uint32_t>(compressed_exe.size()));
  WriteInt32L(&header[12], compressed_exe_crc32);
  return header;
}

}  // namespace saptapper
