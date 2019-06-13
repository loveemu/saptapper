/// @file
/// Byte I/O templates.

#ifndef SAPTAPPER_BYTE_IO_HPP_
#define SAPTAPPER_BYTE_IO_HPP_

#include <cstdint>

namespace saptapper {

/// Writes an 8-bit integer.
/// @param out the output iterator.
/// @param value the number to be written.
/// @return the output iterator that points to the next element of the written
/// data.
/// @tparam OutputIterator an Iterator that can write to the pointed-to element.
template <typename OutputIterator>
constexpr OutputIterator WriteInt8(OutputIterator out, std::uint8_t value) {
  static_assert(sizeof(*out) == 1, "Element size of OutputIterator must be 1.");

  *out = static_cast<char>(value);
  std::advance(out, 1);

  return out;
}

/// Writes a 16-bit integer in little-endian order.
/// @param out the output iterator.
/// @param value the number to be written.
/// @return the output iterator that points to the next element of the written
/// data.
/// @tparam OutputIterator an Iterator that can write to the pointed-to element.
template <typename OutputIterator>
constexpr OutputIterator WriteInt16L(OutputIterator out, std::uint16_t value) {
  static_assert(sizeof(*out) == 1, "Element size of OutputIterator must be 1.");

  *out = value & 0xff;
  std::advance(out, 1);
  *out = static_cast<char>((value >> 8) & 0xff);
  std::advance(out, 1);

  return out;
}

/// Writes a 32-bit integer in little-endian order.
/// @param out the output iterator.
/// @param value the number to be written.
/// @return the output iterator that points to the next element of the written
/// data.
/// @tparam OutputIterator an Iterator that can write to the pointed-to element.
template <typename OutputIterator>
constexpr OutputIterator WriteInt32L(OutputIterator out, std::uint32_t value) {
  static_assert(sizeof(*out) == 1, "Element size of OutputIterator must be 1.");

  *out = static_cast<char>(value & 0xff);
  std::advance(out, 1);
  *out = static_cast<char>((value >> 8) & 0xff);
  std::advance(out, 1);
  *out = static_cast<char>((value >> 16) & 0xff);
  std::advance(out, 1);
  *out = static_cast<char>((value >> 24) & 0xff);
  std::advance(out, 1);

  return out;
}

/// Reads a 8-bit integer in little-endian order.
/// @param in the input iterator.
/// @return the number to be read.
/// @tparam InputIterator an Iterator that can read from the pointed-to element.
template <typename InputIterator>
constexpr std::uint8_t ReadInt8L(InputIterator in) {
  static_assert(sizeof(*in) == 1, "Element size of InputIterator must be 1.");
  return *in;
}

/// Reads a 16-bit integer in little-endian order.
/// @param in the input iterator.
/// @return the number to be read.
/// @tparam InputIterator an Iterator that can read from the pointed-to element.
template <typename InputIterator>
constexpr std::uint16_t ReadInt16L(InputIterator in) {
  static_assert(sizeof(*in) == 1, "Element size of InputIterator must be 1.");

  const std::uint8_t v1 = *in;
  std::advance(in, 1);
  const std::uint8_t v2 = *in;

  return v1 | (v2 << 8);
}

/// Reads a 32-bit integer in little-endian order.
/// @param in the input iterator.
/// @return the number to be read.
/// @tparam InputIterator an Iterator that can read from the pointed-to element.
template <typename InputIterator>
constexpr std::uint32_t ReadInt32L(InputIterator in) {
  static_assert(sizeof(*in) == 1, "Element size of InputIterator must be 1.");

  const std::uint8_t v1 = *in;
  std::advance(in, 1);
  const std::uint8_t v2 = *in;
  std::advance(in, 1);
  const std::uint8_t v3 = *in;
  std::advance(in, 1);
  const std::uint8_t v4 = *in;

  return v1 | (v2 << 8) | (v3 << 16) | (v4 << 24);
}

}  // namespace saptapper

#endif
