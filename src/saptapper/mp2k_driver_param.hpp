/// @file
/// Header file for SoundDriverParam class.

#ifndef MP2K_DRIVER_PARAM_HPP_
#define MP2K_DRIVER_PARAM_HPP_

#include "types.hpp"

namespace saptapper {

class Mp2kDriverParam
{
public:
  Mp2kDriverParam() = default;

  bool ok() const noexcept {
    return song_count_ != 0 && init_fn_ != agbnullptr &&
           main_fn_ != agbnullptr && vsync_fn_ != agbnullptr &&
           select_song_fn_ != agbnullptr;
  }

  int song_count() const noexcept { return song_count_; }

  agbptr_t minigsf_address() const noexcept { return minigsf_address_; }

  agbptr_t minigsf_size() const noexcept { return minigsf_size_; }

  agbptr_t song_table() const noexcept { return song_table_; }

  agbptr_t init_fn() const noexcept { return init_fn_; }

  agbptr_t main_fn() const noexcept { return main_fn_; }

  agbptr_t vsync_fn() const noexcept { return vsync_fn_; }

  agbptr_t select_song_fn() const noexcept { return select_song_fn_; }

  void set_song_count(int count) noexcept { song_count_ = count; }

  void set_minigsf_address(agbptr_t address) noexcept {
    minigsf_address_ = address;
  }

  void set_minigsf_size(agbptr_t size) noexcept {
    minigsf_size_ = size;
  }

  void set_song_table(agbptr_t address) noexcept { song_table_ = address; }

  void set_init_fn(agbptr_t address) noexcept { init_fn_ = address; }

  void set_main_fn(agbptr_t address) noexcept { main_fn_ = address; }

  void set_vsync_fn(agbptr_t address) noexcept { vsync_fn_ = address; }

  void set_select_song_fn(agbptr_t address) noexcept { select_song_fn_ = address; }

 private:
  int song_count_ = 0;
  agbptr_t minigsf_address_ = agbnullptr;
  agbsize_t minigsf_size_ = 0;
  agbptr_t song_table_ = agbnullptr;
  agbptr_t init_fn_ = agbnullptr;
  agbptr_t main_fn_ = agbnullptr;
  agbptr_t vsync_fn_ = agbnullptr;
  agbptr_t select_song_fn_ = agbnullptr;
};

}  // namespace saptapper

#endif
