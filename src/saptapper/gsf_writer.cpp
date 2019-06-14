// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "gsf_writer.hpp"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <utility>
#include "gsf_header.hpp"
#include "psf_writer.hpp"
#include "types.hpp"

namespace saptapper {

void GsfWriter::SaveToFile(const std::filesystem::path& path,
                           const GsfHeader& header, std::string_view rom,
                           std::map<std::string, std::string> tags) {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  file.exceptions(std::ios::badbit);
  SaveToStream(file, header, rom, std::move(tags));
  file.close();
}

void GsfWriter::SaveToStream(std::ostream& out, const GsfHeader& header,
                             std::string_view rom,
                             std::map<std::string, std::string> tags) {
  PsfWriter psf{kVersion, std::move(tags)};
  auto& exe = psf.exe();
  exe.write(header.data(), header.size());
  exe.write(rom.data(), rom.size());
  psf.SaveToStream(out);
}

void GsfWriter::SaveMinigsfToFile(const std::filesystem::path& path,
                                  const MinigsfDriverParam& param,
                                  std::uint32_t song,
                                  std::map<std::string, std::string> tags) {
  std::ofstream file(path, std::ios::out | std::ios::binary);
  file.exceptions(std::ios::badbit);
  SaveMinigsfToStream(file, param, song, std::move(tags));
  file.close();
}

void GsfWriter::SaveMinigsfToStream(std::ostream& out,
                                    const MinigsfDriverParam& param,
                                    std::uint32_t song,
                                    std::map<std::string, std::string> tags) {
  const agbptr_t entrypoint =
      is_romptr(param.address()) ? 0x8000000 : param.address() & 0xff000000;
  const GsfHeader gsf_header{entrypoint, param.address(), param.size()};

  char rom_data[4];
  WriteInt32L(rom_data, song);
  const std::string_view rom{rom_data, param.size()};

  SaveToStream(out, gsf_header, rom, std::move(tags));
}

}  // namespace saptapper
