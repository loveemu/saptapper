/// @file
/// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "saptapper.hpp"

#include <cstring>
#include <iostream>
#include <string_view>
#include "arm.hpp"
#include "bytes.hpp"
#include "byte_pattern.hpp"
#include "cartridge.hpp"
#include "files.hpp"
#include "minigsf_driver_param.hpp"
#include "mp2k_driver.hpp"
#include "mp2k_driver_param.hpp"

namespace saptapper {

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
  agbptr_t addr = FindFreeSpace(rom, size, (char)0xff, largest);
  if (addr == agbnullptr) {
    addr = FindFreeSpace(rom, size, 0, largest);
  }
  return addr;
}

void Saptapper::Inspect(Cartridge& cartridge) const {
  Mp2kDriver driver;
  Mp2kDriverParam param = driver.Inspect(cartridge);

  agbptr_t gsf_driver_addr =
    FindFreeSpace(cartridge.rom(), driver.gsf_driver_size());

  MinigsfDriverParam minigsf;
  minigsf.set_address(driver.minigsf_address(gsf_driver_addr));
  minigsf.set_size(GetMinigsfSize(param.song_count()));

  std::cout << "Status: " << (param.ok() ? "OK" : "FAILED") << std::endl
            << std::endl;

  param.WriteAsTable(std::cout);
  std::cout << std::endl;

  std::cout << "minigsf information:" << std::endl << std::endl;
  minigsf.WriteAsTable(std::cout);

  driver.InstallGsfDriver(cartridge.rom(), gsf_driver_addr, param);
  char nums[4];
  WriteInt32L(nums, 1);
  std::memcpy(cartridge.rom().data() + to_offset(minigsf.address()), nums, minigsf.size());
  WriteAllBytesToFile("test.gsf.gba", cartridge.rom());
}

}  // namespace saptapper
