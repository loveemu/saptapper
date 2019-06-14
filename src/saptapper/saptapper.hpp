// Saptapper: Automated GSF ripper for MusicPlayer2000.

#ifndef SAPTAPPER_SAPTAPPER_HPP_
#define SAPTAPPER_SAPTAPPER_HPP_

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include "cartridge.hpp"
#include "minigsf_driver_param.hpp"
#include "mp2k_driver_param.hpp"
#include "types.hpp"

namespace saptapper {

class Saptapper {
 public:
  static void ConvertToGsfSet(Cartridge& cartridge,
                              const std::filesystem::path& basename,
                              const std::filesystem::path& outdir = "",
                              const std::string_view& gsfby = "");

  static void SaveMinigsfFiles(const std::filesystem::path& base_path,
                               const MinigsfDriverParam& minigsf,
                               int song_count,
                               const std::map<std::string, std::string>& tags);

  static void Inspect(const Cartridge& cartridge, Mp2kDriverParam& param,
                      MinigsfDriverParam& minigsf, agbptr_t& gsf_driver_addr,
                      bool throw_if_missing = false);

  static void PrintParam(const Mp2kDriverParam& param,
                         const MinigsfDriverParam& minigsf);

 private:
  static agbptr_t FindFreeSpace(std::string_view rom, agbsize_t size);
  static agbptr_t FindFreeSpace(std::string_view rom, agbsize_t size,
                                char filler, bool largest);

  static constexpr agbsize_t GetMinigsfSize(int song_count) {
    if (song_count <= 0) return 0;

    agbsize_t size = 1;
    int remaining = song_count >> 8;
    while (size < 4 && remaining != 0) {
      size++;
      remaining >>= 8;
    };
    return size;
  }
};

}  // namespace saptapper

#endif
