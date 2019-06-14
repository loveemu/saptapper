// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "saptapper.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string_view>
#include "cartridge.hpp"
#include "gsf_header.hpp"
#include "gsf_writer.hpp"
#include "minigsf_driver_param.hpp"
#include "mp2k_driver.hpp"
#include "mp2k_driver_param.hpp"

namespace saptapper {

void Saptapper::ConvertToGsfSet(Cartridge& cartridge,
                                const std::filesystem::path& basename,
                                const std::filesystem::path& outdir,
                                const std::string_view& gsfby) {
  Mp2kDriver driver;
  Mp2kDriverParam param = driver.Inspect(cartridge.rom());
  if (!param.ok()) {
    std::ostringstream message;
    message << "Identification of MusicPlayer2000 driver is incomplete."
            << std::endl
            << std::endl;
    (void)param.WriteAsTable(message);
    throw std::runtime_error(message.str());
  }

  const agbptr_t gsf_driver_addr =
      FindFreeSpace(cartridge.rom(), driver.gsf_driver_size());
  if (gsf_driver_addr == agbnullptr) {
    std::ostringstream message;
    message << "Unable to find the free space for gsf driver block ("
            << driver.gsf_driver_size() << " bytes required).";
    throw std::runtime_error(message.str());
  }

  MinigsfDriverParam minigsf;
  minigsf.set_address(driver.minigsf_address(gsf_driver_addr));
  minigsf.set_size(GetMinigsfSize(param.song_count()));

  driver.InstallGsfDriver(cartridge.rom(), gsf_driver_addr, param);

  std::filesystem::path basepath{outdir};
  basepath /= basename;
  create_directories(basepath.parent_path());

  std::filesystem::path gsflib_path{basepath};
  gsflib_path += ".gsflib";

  const agbptr_t entrypoint = 0x8000000;
  const GsfHeader gsf_header{entrypoint, entrypoint, cartridge.size()};
  GsfWriter::SaveToFile(gsflib_path, gsf_header, cartridge.rom());

  for (int song = 0; song < param.song_count(); song++) {
    std::ostringstream songid;
    songid << std::setfill('0') << std::setw(4) << song;

    std::filesystem::path minigsf_path{basepath};
    minigsf_path += "-";
    minigsf_path += songid.str();
    minigsf_path += ".minigsf";

    std::map<std::string, std::string> minigsf_tags{
        {"_lib", gsflib_path.filename().string()}};
    if (!gsfby.empty()) minigsf_tags["gsfby"] = gsfby;

    GsfWriter::SaveMinigsfToFile(minigsf_path, minigsf, song,
                                 std::move(minigsf_tags));
  }
}

void Saptapper::Inspect(const Cartridge& cartridge) {
  Mp2kDriver driver;
  Mp2kDriverParam param = driver.Inspect(cartridge.rom());

  const agbptr_t gsf_driver_addr =
      FindFreeSpace(cartridge.rom(), driver.gsf_driver_size());

  MinigsfDriverParam minigsf;
  minigsf.set_address(driver.minigsf_address(gsf_driver_addr));
  minigsf.set_size(GetMinigsfSize(param.song_count()));

  std::cout << "Status: " << (param.ok() ? "OK" : "FAILED") << std::endl
            << std::endl;

  (void)param.WriteAsTable(std::cout);
  std::cout << std::endl;

  std::cout << "minigsf information:" << std::endl << std::endl;
  (void)minigsf.WriteAsTable(std::cout);
}

agbptr_t Saptapper::FindFreeSpace(std::string_view rom, agbsize_t size,
                                  char filler, bool largest) {
  agbptr_t space = 0;
  agbsize_t space_size = 0;
  for (agbsize_t offset = 0; offset < rom.size(); offset += 4) {
    if (rom[offset] == filler) {
      agbsize_t end_pos = offset + 1;
      while (end_pos < rom.size() && rom[end_pos] == filler) end_pos++;

      const agbsize_t candidate_size = end_pos - offset;
      if (candidate_size >= size) {
        if (candidate_size > space_size) {
          space = to_romptr(offset);
          space_size = candidate_size;
          if (!largest) return space;
        }
      }

      offset = (end_pos + 3) & ~3;
    }
  }
  return space;
}

agbptr_t Saptapper::FindFreeSpace(std::string_view rom, agbsize_t size) {
  constexpr bool largest = false;
  agbptr_t addr = FindFreeSpace(rom, size, '\xff', largest);
  if (addr == agbnullptr) {
    addr = FindFreeSpace(rom, size, 0, largest);
  }
  return addr;
}

}  // namespace saptapper
