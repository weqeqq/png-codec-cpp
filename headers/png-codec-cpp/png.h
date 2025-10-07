
#pragma once

/* clang-format off */
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef PNG_CODEC_SHARED
    #ifdef PNG_CODEC_BUILD
      #define PNG_CODEC_EXPORT __declspec(dllexport)
    #else
      #define PNG_CODEC_EXPORT __declspec(dllimport)
    #endif
  #else
    #define PNG_CODEC_EXPORT
  #endif
#else
  #ifdef PNG_CODEC_SHARED
    #define PNG_CODEC_EXPORT __attribute__((visibility("default")))
  #else
    #define PNG_CODEC_EXPORT
  #endif
#endif
/* clang-format on */

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace PngCodec {

struct Error : std::runtime_error {
  Error(const std::string &message)
      : std::runtime_error("PngCodecError: " + message) {}
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

PNG_CODEC_EXPORT std::vector<std::uint8_t> Decode(const std::uint8_t *data,
                                                  std::size_t length,
                                                  std::size_t &row_count,
                                                  std::size_t &column_count,
                                                  Color color);

inline std::vector<std::uint8_t> Decode(const std::vector<std::uint8_t> &data,
                                        std::size_t &row_count,
                                        std::size_t &column_count,
                                        Color color) {
  return Decode(data.data(), data.size(), row_count, column_count, color);
}

PNG_CODEC_EXPORT std::vector<std::uint8_t> Encode(const std::uint8_t *data,
                                                  std::size_t row_count,
                                                  std::size_t column_count,
                                                  Color color);

inline std::vector<std::uint8_t> Encode(const std::vector<std::uint8_t> &data,
                                        std::size_t row_count,
                                        std::size_t column_count, Color color) {
  return Encode(data.data(), row_count, column_count, color);
}

}  // namespace PngCodec
