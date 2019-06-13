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
  Mp2kDriver driver;
  Mp2kDriverParam param = driver.Inspect(cartridge);
  driver.PrintParam(param);
}

}  // namespace saptapper
