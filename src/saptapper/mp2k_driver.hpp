/// @file
/// Header file for Mp2kDriver class.

#ifndef MP2K_DRIVER_HPP_
#define MP2K_DRIVER_HPP_

#include <string>
#include <string_view>
#include "cartridge.hpp"
#include "mp2k_driver_param.hpp"

namespace saptapper {

class Mp2kDriver {
 public:
  Mp2kDriver() = default;

  std::string name() const { return "MusicPlayer2000"; }

  Mp2kDriverParam Inspect(const Cartridge& cartridge) const;
  void PrintParam(const Mp2kDriverParam& param) const;

 private:
  static agbptr_t FindInitFn(std::string_view rom, agbptr_t main_fn);
  static agbptr_t FindMainFn(std::string_view rom, agbptr_t select_song_fn);
  static agbptr_t FindVSyncFn(std::string_view rom, agbptr_t init_fn);
  static agbptr_t FindSelectSongFn(std::string_view rom);
  static agbptr_t FindSongTable(std::string_view rom, agbptr_t select_song_fn);
  static int ReadSongCount(std::string_view rom, agbptr_t song_table);
};

}  // namespace saptapper

#endif
