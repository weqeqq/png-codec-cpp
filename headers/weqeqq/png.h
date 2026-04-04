#pragma once

#include <weqeqq/color.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <ranges>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace weqeqq::png {

using Color = color::Format;

inline constexpr std::array<std::uint8_t, 8> Signature = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

struct Error : std::runtime_error {
  Error(std::string_view message,
        std::source_location location = std::source_location::current())
      : std::runtime_error(std::format("[{}:{}] {}", location.file_name(),
                                       location.line(), message)) {}
};

struct ErrorInvalidInputData : Error {
  explicit ErrorInvalidInputData(
      std::source_location location = std::source_location::current())
      : Error("Invalid input data", location) {}
};

struct ErrorInvalidPngSignature : Error {
  explicit ErrorInvalidPngSignature(
      std::source_location location = std::source_location::current())
      : Error("Invalid PNG signature", location) {}
};

struct ErrorInvalidInputParameters : Error {
  explicit ErrorInvalidInputParameters(
      std::source_location location = std::source_location::current())
      : Error("Invalid input parameters", location) {}
};

struct ErrorInvalidImageDimensions : Error {
  ErrorInvalidImageDimensions(
      std::size_t width, std::size_t height,
      std::source_location location = std::source_location::current())
      : Error(std::format("Invalid image dimensions: {}x{}", width, height),
              location) {}
};

struct ErrorWuffsPngDecoderAllocationFailed : Error {
  explicit ErrorWuffsPngDecoderAllocationFailed(
      std::source_location location = std::source_location::current())
      : Error("Failed to allocate PNG decoder", location) {}
};

struct ErrorWuffsPngDecoderInitializationFailed : Error {
  ErrorWuffsPngDecoderInitializationFailed(
      std::string_view detail,
      std::source_location location = std::source_location::current())
      : Error(std::format("Failed to initialize PNG decoder: {}", detail),
              location) {}
};

struct ErrorWuffsPngImageConfigDecodeFailed : Error {
  ErrorWuffsPngImageConfigDecodeFailed(
      std::string_view detail,
      std::source_location location = std::source_location::current())
      : Error(std::format("Failed to decode PNG image config: {}", detail),
              location) {}
};

struct ErrorWuffsPixelBufferSetupFailed : Error {
  ErrorWuffsPixelBufferSetupFailed(
      std::string_view detail,
      std::source_location location = std::source_location::current())
      : Error(std::format("Failed to set pixel buffer: {}", detail), location) {
  }
};

struct ErrorWuffsPngFrameDecodeFailed : Error {
  ErrorWuffsPngFrameDecodeFailed(
      std::string_view detail,
      std::source_location location = std::source_location::current())
      : Error(std::format("Failed to decode PNG frame: {}", detail), location) {
  }
};

struct ErrorLodepngDecodingFailed : Error {
  ErrorLodepngDecodingFailed(
      unsigned code, std::string_view detail,
      std::source_location location = std::source_location::current())
      : Error(
            std::format("lodepng decoding failed (code {}): {}", code, detail),
            location) {}
};

struct ErrorLodepngEncodingFailed : Error {
  ErrorLodepngEncodingFailed(unsigned code, std::source_location location =
                                                std::source_location::current())
      : Error(std::format("lodepng encoding failed (code {})", code),
              location) {}
};

struct ErrorLodepngEmptyOutput : Error {
  explicit ErrorLodepngEmptyOutput(
      std::source_location location = std::source_location::current())
      : Error("lodepng produced empty output", location) {}
};

struct ErrorFpngEncodingFailed : Error {
  explicit ErrorFpngEncodingFailed(
      std::source_location location = std::source_location::current())
      : Error("fpng encoding failed", location) {}
};

struct ErrorFpngEmptyOutput : Error {
  explicit ErrorFpngEmptyOutput(
      std::source_location location = std::source_location::current())
      : Error("fpng produced empty output", location) {}
};

struct ImageInfo {
  std::size_t width;
  std::size_t height;
  Color color;
};

struct DecodedImage {
  std::vector<std::uint8_t> data;
  ImageInfo info;
};

template <typename R>
  requires std::ranges::input_range<R> &&
           std::same_as<std::ranges::range_value_t<R>, std::uint8_t>
constexpr bool HasPngSignature(R&& range) {
  auto iterator = std::ranges::begin(range);
  auto end = std::ranges::end(range);

  if (static_cast<std::size_t>(std::ranges::distance(iterator, end)) <
      Signature.size()) {
    return false;
  }

  return std::ranges::equal(
      Signature, std::ranges::subrange(
                     iterator, std::ranges::next(iterator, Signature.size())));
}

constexpr bool HasPngSignature(std::span<const std::uint8_t> data) {
  if (data.size() < Signature.size()) {
    return false;
  }
  return std::ranges::equal(Signature, data.first<Signature.size()>());
}

WQPNG_EXPORT DecodedImage DecodeImage(std::span<const std::uint8_t> data,
                                      Color color);

WQPNG_EXPORT std::vector<std::uint8_t> EncodeImage(
    std::span<const std::uint8_t> data, std::size_t width, std::size_t height,
    Color color);

}  // namespace weqeqq::png
