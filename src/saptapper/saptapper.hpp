/// @file
/// Header file for Saptapper class.

#ifndef SAPTAPPER_HPP_
#define SAPTAPPER_HPP_

#include "cartridge.hpp"

namespace saptapper {

class Saptapper {
 public:
  Saptapper() = default;

  void Inspect(const Cartridge& cartridge) const;
};

}  // namespace saptapper

#endif
