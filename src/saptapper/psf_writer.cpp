#include "psf_writer.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include "bytes.hpp"

namespace saptapper {

PsfWriter::PsfWriter(uint8_t version)
    : version_{version}, exe_{compressed_exe_, zstr::default_buff_size, Z_BEST_COMPRESSION, 15} {}

void PsfWriter::SaveToFile(const std::filesystem::path& path) {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  file.exceptions(std::ios::badbit);
  SaveToStream(file);
  file.close();
}

void PsfWriter::SaveToStream(std::ostream& out) {
  exe_.flush();
  reserved_.flush();

  const std::string compressed_exe = compressed_exe_.str();
  const std::string reserved = reserved_.str();
  const std::uint32_t compressed_exe_crc32 =
      crc32(0L, reinterpret_cast<const Bytef*>(compressed_exe.data()),
            static_cast<uInt>(compressed_exe.size()));

  const std::string header{NewHeader(compressed_exe, reserved, compressed_exe_crc32)};
  out.write(header.data(), header.size());
  out.write(reserved.data(), reserved.size());
  out.write(compressed_exe.data(), compressed_exe.size());
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
