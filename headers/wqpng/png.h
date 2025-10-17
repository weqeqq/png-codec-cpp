
#pragma once

#include <array>
#include <iterator>

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef PNG_SHARED
#ifdef PNG_BUILD
#define PNG_EXPORT __declspec(dllexport)
#else
#define PNG_EXPORT __declspec(dllimport)
#endif
#else
#define PNG_EXPORT
#endif
#else
#ifdef PNG_SHARED
#define PNG_EXPORT __attribute__((visibility("default")))
#else
#define PNG_EXPORT
#endif
#endif

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace Png {

inline constexpr std::array<std::uint8_t, 8> Signature = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

struct Error : std::runtime_error {
  Error(const std::string &message) : std::runtime_error("Png: " + message) {}
};

struct DecodeError : Error {
  using Error::Error;
};

struct EncodeError : Error {
  using Error::Error;
};

enum class Color : std::uint32_t {
  Bgr = 0x80000888,
  Bgra = 0x81008888,
  Bgrx = 0x90008888,
  Rgb = 0xA0000888,
  Rgba = 0xA1008888,
  Rgbx = 0xB0008888,
  Cmyk = 0xD0038888,
};

template <typename I>
inline bool IsPng(I begin, I end) {
  if (static_cast<std::size_t>(std::distance(begin, end)) < Signature.size()) {
    return false;
  }
  for (auto index = 0u; index < Signature.size(); ++index) {
    if (*begin++ != Signature[index]) {
      return false;
    }
  }
  return true;
}

template <typename T>
inline bool IsPng(T data) {
  return IsPng(data.begin(), data.end());
}

PNG_EXPORT std::vector<std::uint8_t> Decode(const std::uint8_t *data,
                                            std::size_t length,
                                            std::size_t &row_count,
                                            std::size_t &column_count,
                                            Color color = Color::Rgba);

inline std::vector<std::uint8_t> Decode(const std::vector<std::uint8_t> &data,
                                        std::size_t &row_count,
                                        std::size_t &column_count,
                                        Color color = Color::Rgba) {
  return Decode(data.data(), data.size(), row_count, column_count, color);
}

PNG_EXPORT std::vector<std::uint8_t> Encode(const std::uint8_t *data,
                                            std::size_t row_count,
                                            std::size_t column_count,
                                            Color color = Color::Rgba);

inline std::vector<std::uint8_t> Encode(const std::vector<std::uint8_t> &data,
                                        std::size_t row_count,
                                        std::size_t column_count,
                                        Color color = Color::Rgba) {
  return Encode(data.data(), row_count, column_count, color);
}

}  // namespace Png
