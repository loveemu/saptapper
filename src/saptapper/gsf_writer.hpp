#ifndef SAPTAPPER_GSF_WRITER_HPP_
#define SAPTAPPER_GSF_WRITER_HPP_

#include <cstdint>
#include <filesystem>
#include <string_view>
#include "gsf_header.hpp"
#include "types.hpp"

namespace saptapper {

class GsfWriter {
 public:
  static void SaveToFile(const std::filesystem::path& path,
                         const GsfHeader& header, std::string_view rom);
  static void SaveToStream(std::ostream& out, const GsfHeader& header,
                           std::string_view rom);

 private:
  static constexpr std::uint8_t kVersion = 0x22;
};

}  // namespace saptapper

#endif
