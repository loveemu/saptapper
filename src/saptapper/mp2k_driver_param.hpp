/// @file
/// Header file for SoundDriverParam class.

#ifndef MP2K_DRIVER_PARAM_HPP_
#define MP2K_DRIVER_PARAM_HPP_

#include <optional>
#include "types.hpp"

namespace saptapper {

class Mp2kDriverParam
{
public:
  static constexpr auto npos{static_cast<agbptr_t>(-1)};

  Mp2kDriverParam() = default;

  bool ok() const noexcept {
    return song_count_ != 0 && minigsf_address_ != npos && minigsf_size_ != 0 &&
           init_fn_ != npos && main_fn_ != npos && vsync_fn_ != npos;
  }

  int song_count() const noexcept { return song_count_; }

  agbptr_t minigsf_address() const noexcept {
    return minigsf_address_;
  }

  agbptr_t minigsf_size() const noexcept {
    return minigsf_size_;
  }

  void set_song_count(int count) noexcept { song_count_ = count; }

  void set_minigsf_address(agbptr_t address) noexcept {
    minigsf_address_ = address;
  }

  void set_minigsf_size(agbptr_t size) noexcept {
    minigsf_size_ = size;
  }

  void set_init_fn(agbptr_t address) noexcept { init_fn_ = address; }

  void set_main_fn(agbptr_t address) noexcept { main_fn_ = address; }

  void set_vsync_fn(agbptr_t address) noexcept { vsync_fn_ = address; }

 private:
  int song_count_ = 0;
  agbptr_t minigsf_address_ = npos;
  agbsize_t minigsf_size_ = 0;
  agbptr_t init_fn_ = npos;
  agbptr_t main_fn_ = npos;
  agbptr_t vsync_fn_ = npos;
};

}  // namespace saptapper

#endif
