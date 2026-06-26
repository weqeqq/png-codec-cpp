module;

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

export module weqeqq.png:types;

export import weqeqq.color;

export namespace weqeqq::png {

using Color = weqeqq::color::Format;

inline constexpr std::array<std::uint8_t, 8> Signature = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

constexpr std::string_view ColorName(Color color) {
  switch (color) {
    case Color::kRgb:
      return "rgb";
    case Color::kRgba:
      return "rgba";
    case Color::kGrayscale:
      return "grayscale";
    case Color::kCmyk:
      return "cmyk";
    default:
      return "unknown";
  }
}

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

DecodedImage DecodeImage(std::span<const std::uint8_t> data, Color color);

std::vector<std::uint8_t> EncodeImage(std::span<const std::uint8_t> data,
                                      std::size_t width, std::size_t height,
                                      Color color);

}  // namespace weqeqq::png
