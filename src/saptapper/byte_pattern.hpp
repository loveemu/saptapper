/// @file
/// Byte pattern class for flexible byte sequence search.
///
/// Heavily inspired by SigScan at GameDeception.net

#ifndef SAPTAPPER_BYTE_PATTERN_
#define SAPTAPPER_BYTE_PATTERN_

#include <stdexcept>
#include <string>
#include <string_view>

namespace saptapper {

class BytePattern
{
public:
  using size_type = std::string::size_type;

  BytePattern(std::string_view data) : data_(data) {}

  BytePattern(std::string_view data, std::string_view mask)
      : data_(data), mask_(mask) {
    if (data.size() != mask.size()) {
      throw std::invalid_argument(
          "BytePattern: data and mask must be the same size");
    }
  }

  size_type size() const noexcept { return data_.size(); }

  bool Match(std::string_view data, size_type pos = 0) const;

  size_type Find(std::string_view data, size_type pos = 0) const;

private:
  /// The pattern to scan for.
  std::string data_;

  /// A mask to ignore certain bytes in the pattern such as addresses.
  ///
  /// The mask should be as long as all the bytes in the pattern.
  /// Use '?' to ignore a byte and 'x' to check it.
  /// Example: "xxx????xx" - The first 3 bytes are checked, then the next 4 are ignored,
  /// then the last 2 are checked
  std::string mask_;
};

} // namespace saptapper

#endif
