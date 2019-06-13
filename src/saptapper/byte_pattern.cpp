#include "byte_pattern.hpp"

#include <string>
#include <string_view>

namespace saptapper {

bool BytePattern::Match(std::string_view data, size_type pos) const {
  if (data.size() < pos + size()) return false;

  for (size_type offset = 0; offset < size(); offset++) {
    if (!mask_.empty() && mask_[offset] == '?') continue;
    if (data[pos + offset] != data_[offset]) return false;
  }
  return true;
}

BytePattern::size_type BytePattern::Find(std::string_view data,
                                         size_type pos) const {
  if (data.size() < pos + size()) return std::string::npos;

  for (size_type offset = pos; offset <= data.size() - size(); offset++) {
    if (Match(data, offset)) {
      return offset;
    }
  }
  return std::string::npos;
}

}  // namespace saptapper
