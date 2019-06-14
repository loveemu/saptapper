// Saptapper: Automated GSF ripper for MusicPlayer2000.

#ifndef SAPTAPPER_GSF_WRITER_HPP_
#define SAPTAPPER_GSF_WRITER_HPP_

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include "gsf_header.hpp"
#include "minigsf_driver_param.hpp"

namespace saptapper {

class GsfWriter {
 public:
  static void SaveToFile(const std::filesystem::path& path,
                         const GsfHeader& header, std::string_view rom,
                         const std::map<std::string, std::string>& tags = {});

  static void SaveToStream(std::ostream& out, const GsfHeader& header,
                           std::string_view rom,
                           const std::map<std::string, std::string>& tags = {});

  static void SaveMinigsfToFile(
      const std::filesystem::path& path, const MinigsfDriverParam& param,
      std::uint32_t song, const std::map<std::string, std::string>& tags = {});

  static void SaveMinigsfToStream(
      std::ostream& out, const MinigsfDriverParam& param, std::uint32_t song,
      const std::map<std::string, std::string>& tags = {});

 private:
  static constexpr std::uint8_t kVersion = 0x22;
};

}  // namespace saptapper

#endif
