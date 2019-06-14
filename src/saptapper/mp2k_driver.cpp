// Saptapper: Automated GSF ripper for MusicPlayer2000.

#include "mp2k_driver.hpp"

#include <array>
#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include "algorithm.hpp"
#include "arm.hpp"
#include "byte_pattern.hpp"
#include "bytes.hpp"
#include "mp2k_driver_param.hpp"
#include "types.hpp"

namespace saptapper {

Mp2kDriverParam Mp2kDriver::Inspect(std::string_view rom) {
  Mp2kDriverParam param;
  param.set_select_song_fn(FindSelectSongFn(rom));
  param.set_song_table(FindSongTable(rom, param.select_song_fn()));
  param.set_main_fn(FindMainFn(rom, param.select_song_fn()));
  param.set_init_fn(FindInitFn(rom, param.main_fn()));
  param.set_vsync_fn(FindVSyncFn(rom, param.init_fn()));
  param.set_song_count(ReadSongCount(rom, param.song_table()));
  return param;
}

void Mp2kDriver::InstallGsfDriver(std::string& rom, agbptr_t address,
                                  const Mp2kDriverParam& param) {
  if (!is_romptr(address))
    throw std::invalid_argument("The gsf driver address is not valid.");
  if (!param.ok()) {
    std::ostringstream message;
    message << "Identification of MusicPlayer2000 driver is incomplete."
            << std::endl << std::endl;
    (void)param.WriteAsTable(message);
    throw std::invalid_argument(message.str());
  }

  agbsize_t offset = to_offset(address);
  if (offset + gsf_driver_size() > rom.size())
    throw std::out_of_range("The address of gsf driver block is out of range.");

  std::memcpy(&rom[offset], gsf_driver_block, gsf_driver_size());
  WriteInt32L(&rom[offset + kInitFnOffset], param.init_fn() | 1);
  WriteInt32L(&rom[offset + kSelectSongFnOffset], param.select_song_fn() | 1);
  WriteInt32L(&rom[offset + kMainFnOffset], param.main_fn() | 1);
  WriteInt32L(&rom[offset + kVSyncFnOffset], param.vsync_fn() | 1);

  WriteInt32L(rom.data(), make_arm_b(0x8000000, address));
}

int Mp2kDriver::FindIdenticalSong(std::string_view rom, agbptr_t song_table,
                                   int song) {
  if (song_table == agbnullptr) return kNoSong;

  agbsize_t start_pos = to_offset(song_table);
  if (start_pos >= rom.size()) return kNoSong;

  agbsize_t target_pos = start_pos + (8 * song);
  if (target_pos + 8 >= rom.size()) return kNoSong;

  int current_song = 0;
  for (agbsize_t pos = start_pos; pos < target_pos; pos += 8, current_song++) {
    if (std::memcmp(&rom[pos], &rom[target_pos], 8) == 0) return current_song;
  }
  return kNoSong;
}

agbptr_t Mp2kDriver::FindInitFn(std::string_view rom, agbptr_t main_fn) {
  if (main_fn == agbnullptr) return agbnullptr;

  using namespace std::literals::string_view_literals;
  std::array patterns = {
      "\x70\xb5"sv,  // push {r4-r6,lr}
      "\xf0\xb5"sv,  // push {r4-r7,lr}};
  };
  return find_backwards(rom, patterns, to_offset(main_fn), 0x100);
}  // namespace saptapper

agbptr_t Mp2kDriver::FindMainFn(std::string_view rom, agbptr_t select_song_fn) {
  if (select_song_fn == agbnullptr) return agbnullptr;

  using namespace std::literals::string_view_literals;
  std::array patterns{"\x00\xb5"sv};  // push lr
  return find_backwards(rom, patterns, to_offset(select_song_fn), 0x20);
}

agbptr_t Mp2kDriver::FindVSyncFn(std::string_view rom, agbptr_t init_fn) {
  if (init_fn == agbnullptr) return agbnullptr;

  using namespace std::literals::string_view_literals;
  // LDR     R0, =dword_3007FF0
  // LDR     R0, [R0]
  // LDR     R2, =0x68736D53
  // LDR     R3, [R0]
  // SUBS (later versions) or CMP (earlier versions, such as Momotarou Matsuri)
  const BytePattern pattern{"\xa6\x48\x00\x68\xa6\x4a\x03\x68"sv, "?xxx?xxx"sv};

  // Pattern for Puyo Pop Fever, Precure, etc.:
  //
  // PUSH    {LR}
  // LDR     R0, =dword_3007FF0
  // LDR     R2, [R0]
  // LDR     R0, [R2]
  // LDR     R1, =0x978C92AD
  const BytePattern pattern2{"\x00\xb5\x18\x48\x02\x68\x10\x68\x17\x49"sv,
                             "xx?xxxxx?x"sv};

  const agbsize_t init_fn_pos = to_offset(init_fn);
  if (init_fn_pos >= rom.size()) return agbnullptr;

  // The m4aSoundVSync function is far from m4aSoundInit.
  constexpr agbsize_t length =
      0x1800;  // 0x1000 might be good, but longer is safer anyway :)
  constexpr agbsize_t align = 4;
  assert(length % align == 0);
  if (rom.size() < length) return agbnullptr;

  // Regular version:
  //
  // Search backwards from m4aSoundInit function.
  const agbsize_t max_pos = init_fn_pos - align;
  const agbsize_t min_pos = init_fn_pos - length;
  for (agbsize_t offset = max_pos; offset >= min_pos; offset -= align) {
    if (pattern.Match(std::string_view{rom.data() + offset, pattern.size()})) {
      // Momotarou Matsuri:
      // check "BX LR" and avoid false-positive
      if (offset + 0x0e >= rom.size() &&
          ReadInt16L(rom.data() + offset + 0x0c) == 0x4770) {
        continue;
      }

      return to_romptr(offset);
    }
  }

  // Alternate version (Puyo Pop Fever, Precure, etc.):
  //
  // Search forwards from m4aSoundInit function.
  const agbsize_t min_pos2 = init_fn_pos + align;
  const agbsize_t max_pos2 = std::min<agbsize_t>(
      init_fn_pos + length, static_cast<agbsize_t>(rom.size()));
  for (agbsize_t offset = min_pos2; offset < max_pos2; offset += align) {
    if (pattern2.Match(std::string_view{rom.data() + offset, pattern2.size()}))
      return to_romptr(offset);
  }

  return agbnullptr;
}

agbptr_t Mp2kDriver::FindSelectSongFn(std::string_view rom) {
  using namespace std::literals::string_view_literals;
  const std::string_view pattern{
      "\x00\xb5\x00\x04\x07\x4a\x08\x49\x40\x0b"sv
      "\x40\x18\x83\x88\x59\x00\xc9\x18\x89\x00"sv
      "\x89\x18\x0a\x68\x01\x68\x10\x1c\x00\xf0"sv};
  return find_loose(rom, pattern, 8);
}

agbptr_t Mp2kDriver::FindSongTable(std::string_view rom,
                                   agbptr_t select_song_fn) {
  if (select_song_fn == agbnullptr) return agbnullptr;

  const agbsize_t select_song_fn_pos = to_offset(select_song_fn);
  if (select_song_fn_pos + 40 + 4 > rom.size()) return agbnullptr;

  const agbptr_t song_table = ReadInt32L(rom.data() + select_song_fn_pos + 40);
  if (!is_romptr(song_table)) return agbnullptr;
  if (to_offset(song_table) >= rom.size()) return agbnullptr;

  return song_table;
}

int Mp2kDriver::ReadSongCount(std::string_view rom, agbptr_t song_table) {
  if (song_table == agbnullptr) return 0;

  const agbsize_t song_table_pos = to_offset(song_table);
  if (rom.size() < 8) return 0;
  if (song_table_pos > rom.size() - 8) return 0;

  int song_count = 0;
  for (agbsize_t offset = song_table_pos; offset <= rom.size() - 8;
       offset += 8) {
    const agbptr_t song = ReadInt32L(rom.data() + offset);
    if (!is_romptr(song)) break;
    song_count++;
  }
  return song_count;
}

}  // namespace saptapper
