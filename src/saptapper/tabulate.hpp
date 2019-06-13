/// @file
/// Tabulate function for fancy output.

#ifndef SAPTAPPER_TABULATE_HPP_
#define SAPTAPPER_TABULATE_HPP_

#include <array>
#include <iomanip>
#include <iostream>

namespace saptapper {

template <class _Ty, size_t _NumOfColumns, size_t _NumOfRows>
static std::ostream& tabulate(
    std::ostream& stream, std::array<_Ty, _NumOfColumns> header,
    std::array<std::array<_Ty, _NumOfColumns>, _NumOfRows> items) {
  // Determine column lengths.
  //
  // Note that it doesn't calculate proper lengths for non-ASCII characters.
  std::array<size_t, _NumOfColumns> maxlength;
  for (size_t i = 0; i < header.size(); i++) {
    maxlength[i] = header[i].size();
  }
  for (const auto& item : items) {
    for (size_t i = 0; i < item.size(); i++) {
      if (maxlength[i] < item[i].size()) maxlength[i] = item[i].size();
    }
  }

  // Header
  stream << std::setfill(' ');
  for (size_t i = 0; i < header.size(); i++) {
    stream << "|" << std::setw(maxlength[i]) << std::left << header[i] << " ";
  }
  stream << "|" << std::endl;

  // Separator
  stream << std::setfill('-');
  for (size_t i = 0; i < header.size(); i++) {
    stream << "|" << std::setw(maxlength[i] + 1) << "";
  }
  stream << "|" << std::endl;
  stream << std::setfill(' ');

  // Columns
  for (const auto& item : items) {
    for (size_t i = 0; i < item.size(); i++) {
      stream << "|" << std::setw(maxlength[i]) << std::left << item[i] << " ";
    }
    stream << "|" << std::endl;
  }

  return stream;
}

}  // namespace saptapper

#endif
