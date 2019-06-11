/// @file
/// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "saptapper.hpp"

#include <cstdio>
#include <iostream>
#include "arm.hpp"
#include "byte_io.hpp"
#include "byte_pattern.hpp"
#include "cartridge.hpp"
#include "mp2k_driver.hpp"

namespace saptapper {

void Saptapper::Inspect(const Cartridge& cartridge) const {
  std::cout << cartridge.size() << " bytes loaded" << std::endl;
  std::cout << cartridge.game_title() << std::endl;
  std::cout << cartridge.game_code() << std::endl;

  const auto& data = cartridge.data();
  const armins_t ins = ReadInt32L(&data[0]);
  if (is_arm_b(ins)) {
    auto aaa = arm_b_dest(0x8000000, ins);
    printf("entrypoint 0x%08X\n", aaa);
  }

  Mp2kDriver driver;
  auto param = driver.Inspect(cartridge);
  if (!param.ok()) {
    std::cout << "Something is missing" << std::endl;
  }
}

}  // namespace saptapper
