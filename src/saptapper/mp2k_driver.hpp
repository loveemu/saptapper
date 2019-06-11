/// @file
/// Header file for Mp2kDriver class.

#ifndef MP2K_DRIVER_HPP_
#define MP2K_DRIVER_HPP_

#include <string>
#include "cartridge.hpp"
#include "mp2k_driver_param.hpp"

namespace saptapper {

class Mp2kDriver
{
public:
  Mp2kDriver() = default;

  std::string name() const { return "MusicPlayer2000"; }

  Mp2kDriverParam Inspect(const Cartridge& cartridge) const {
    Mp2kDriverParam param;
    param.set_minigsf_address(777);
    return param;
  }
};

}  // namespace saptapper

#endif
