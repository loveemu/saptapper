#include "gsf_writer.hpp"

#include <filesystem>
#include <fstream>
#include <string_view>
#include "gsf_header.hpp"
#include "psf_writer.hpp"

namespace saptapper {

void GsfWriter::SaveToFile(const std::filesystem::path& path,
                           const GsfHeader& header, std::string_view rom) {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  file.exceptions(std::ios::badbit);
  SaveToStream(file, header, rom);
  file.close();
}

void GsfWriter::SaveToStream(std::ostream& out, const GsfHeader& header,
                             std::string_view rom) {
  PsfWriter psf{kVersion};
  auto& exe = psf.exe();
  exe.write(header.data(), header.size());
  exe.write(rom.data(), rom.size());
  psf.SaveToStream(out);
}

}  // namespace saptapper
