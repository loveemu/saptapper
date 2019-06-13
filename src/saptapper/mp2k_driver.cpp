/// @file
/// Source file for Mp2kDriver class.

#include "mp2k_driver.hpp"

#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include "algorithm.hpp"
#include "byte_io.hpp"
#include "byte_pattern.hpp"
#include "types.hpp"
#include "tabulate.hpp"

namespace saptapper {

Mp2kDriverParam Mp2kDriver::Inspect(const Cartridge& cartridge) const {
  std::string_view rom{cartridge.rom()};

  Mp2kDriverParam param;
  param.set_select_song_fn(FindSelectSongFn(rom));
  param.set_song_table(FindSongTable(rom, param.select_song_fn()));
  param.set_main_fn(FindMainFn(rom, param.select_song_fn()));
  param.set_init_fn(FindInitFn(rom, param.main_fn()));
  param.set_vsync_fn(FindVSyncFn(rom, param.init_fn()));
  param.set_song_count(ReadSongCount(rom, param.song_table()));

  return param;
}

agbptr_t Mp2kDriver::FindInitFn(std::string_view rom, agbptr_t main_fn) {
  if (main_fn == agbnullptr) return agbnullptr;

  std::array patterns = {
      std::string_view{"\x70\xb5", 2},  // push {r4-r6,lr}
      std::string_view{"\xf0\xb5", 2},  // push {r4-r7,lr}
  };
  return find_backwards(rom, patterns, to_offset(main_fn), 0x100);
}

agbptr_t Mp2kDriver::FindMainFn(std::string_view rom,
                                agbptr_t select_song_fn) {
  if (select_song_fn == agbnullptr) return agbnullptr;

  std::array patterns{
      std::string_view{"\x00\xb5", 2}  // push lr
  };
  return find_backwards(rom, patterns, to_offset(select_song_fn), 0x20);
}

agbptr_t Mp2kDriver::FindVSyncFn(std::string_view rom, agbptr_t init_fn) {
  if (init_fn == agbnullptr) return agbnullptr;

  // LDR     R0, =dword_3007FF0
  // LDR     R0, [R0]
  // LDR     R2, =0x68736D53
  // LDR     R3, [R0]
  // SUBS (later versions) or CMP (earlier versions, such as Momotarou Matsuri)
  const BytePattern pattern{
      std::string_view{"\xa6\x48\x00\x68\xa6\x4a\x03\x68      ", 14},
      std::string_view{"?xxx?xxx??????", 14}};

  // Pattern for Puyo Pop Fever, Precure, etc.:
  //
  // PUSH    {LR}
  // LDR     R0, =dword_3007FF0
  // LDR     R2, [R0]
  // LDR     R0, [R2]
  // LDR     R1, =0x978C92AD
  const BytePattern pattern2{
      std::string_view{"\x00\xb5\x18\x48\x02\x68\x10\x68\x17\x49", 10},
      std::string_view{"xx?xxxxx?x", 10}};

  agbsize_t init_fn_pos = to_offset(init_fn);
  if (init_fn_pos >= rom.size()) return agbnullptr;

  // The m4aSoundVSync function is far from m4aSoundInit.
  constexpr agbsize_t length = 0x1800;  // 0x1000 might be good, but longer is safer anyway :)
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
      if (ReadInt16L(rom.data() + offset + 0x0c) == 0x4770) continue;

      return to_romptr(offset);
    }
  }

  // Alternate version (Puyo Pop Fever, Precure, etc.):
  //
  // Search forwards from m4aSoundInit function.
  const agbsize_t min_pos2 = init_fn_pos + align;
  const agbsize_t max_pos2 = std::min<agbsize_t>(
      init_fn_pos + length, static_cast<agbsize_t>(rom.size()));
  for (agbsize_t offset = min_pos; offset < max_pos; offset += align) {
    if (pattern2.Match(std::string_view{rom.data() + offset, pattern2.size()}))
      return to_romptr(offset);
  }

  return agbnullptr;
}

agbptr_t Mp2kDriver::FindSelectSongFn(std::string_view rom) {
  const std::string_view pattern{
      "\x00\xb5\x00\x04\x07\x4a\x08\x49\x40\x0b"
      "\x40\x18\x83\x88\x59\x00\xc9\x18\x89\x00"
      "\x89\x18\x0a\x68\x01\x68\x10\x1c\x00\xf0",
      0x1e};
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
    agbptr_t song = ReadInt32L(rom.data() + offset);
    if (!is_romptr(song)) break;
    song_count++;
  }
  return song_count;
}

void Mp2kDriver::PrintParam(const Mp2kDriverParam& param) const {
  std::ostream& stream = std::cout;
  stream << "Status: " << (param.ok() ? "OK" : "FAILED") << std::endl
         << std::endl;

  using row_t = std::array<std::string, 2>;
  const row_t header{"Name", "Address / Value"};
  std::array items{
      row_t{"m4aSoundVSync", to_string(param.vsync_fn())},
      row_t{"m4aSoundInit", to_string(param.init_fn())},
      row_t{"m4aSoundMain", to_string(param.main_fn())},
      row_t{"m4aSongNumStart", to_string(param.select_song_fn())},
      row_t{"song_table", to_string(param.song_table())},
      row_t{"len(song_table)", std::to_string(param.song_count())},
  };
  tabulate(stream, header, items);
}

}  // namespace saptapper
