// Saptapper: Automated GSF ripper for MusicPlayer2000.

#ifndef SAPTAPPER_MINIGSF_DRIVER_PARAM_HPP_
#define SAPTAPPER_MINIGSF_DRIVER_PARAM_HPP_

#include <array>
#include <iostream>
#include <string>
#include "tabulate.hpp"
#include "types.hpp"

namespace saptapper {

class MinigsfDriverParam {
 public:
  MinigsfDriverParam() = default;

  bool ok() const noexcept {
    return address_ != agbnullptr && size_ != 0;
  }

  agbptr_t address() const noexcept { return address_; }

  agbptr_t size() const noexcept { return size_; }

  void set_address(agbptr_t address) noexcept {
    address_ = address;
  }

  void set_size(agbptr_t size) noexcept { size_ = size; }

  std::ostream& WriteAsTable(std::ostream& stream) const {
    using row_t = std::array<std::string, 2>;
    const row_t header{"Name", "Address / Value"};
    std::array items{
        row_t{"minigsf address", to_string(this->address())},
        row_t{"minigsf size", std::to_string(this->size())},
    };

    tabulate(stream, header, items);
    return stream;
  }

 private:
  agbptr_t address_ = agbnullptr;
  agbsize_t size_ = 0;
};

}  // namespace saptapper

#endif
