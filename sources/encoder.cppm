module;

#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <vector>

#include <fpng.h>

module weqeqq.png:encoder;

import :types;
import :error;
import weqeqq.color;

namespace weqeqq::png {
namespace {

class FpngInit {
 public:
  FpngInit() {
    if (!init_) {
      fpng::fpng_init();
      init_ = true;
    }
  }

 private:
  static inline bool init_ = false;
};

}  // namespace

std::vector<std::uint8_t> EncodeImage(std::span<const std::uint8_t> data,
                                      std::size_t width, std::size_t height,
                                      Color color) {
  if (width == 0 || height == 0 || data.empty()) {
    throw error::TypedErrorBuilder(PngError::kInvalidInputParameters,
                                   "Cannot encode an empty image")
        .Field("width", width)
        .Field("height", height)
        .Field("color", std::string(ColorName(color)))
        .Field("data_size", data.size())
        .Hint(
            "Provide a non-empty pixel buffer together with a non-zero width "
            "and height.")
        .Build();
  }

  const std::size_t channel_count = color::ChannelCount(color);
  const std::size_t expected_size = width * height * channel_count;
  if (data.size() != expected_size) {
    throw error::TypedErrorBuilder(
            PngError::kInvalidInputParameters,
            "Pixel buffer size does not match the image dimensions")
        .Field("width", width)
        .Field("height", height)
        .Field("color", std::string(ColorName(color)))
        .Field("channels_per_pixel", channel_count)
        .Field("expected_size", expected_size)
        .Field("actual_size", data.size())
        .Details(std::format(
            "Encoding expects width * height * channels = {} * {} * {} = {} "
            "bytes, but the buffer holds {}.",
            width, height, channel_count, expected_size, data.size()))
        .Hint(
            "Resize the buffer to hold exactly one pixel per coordinate in the "
            "selected color format.")
        .Build();
  }

  static FpngInit fpng_init;

  std::vector<std::uint8_t> converted_buffer;
  std::uint32_t fpng_channel_count = 0;
  switch (color) {
    case Color::kRgba:
      fpng_channel_count = 4;
      break;
    case Color::kRgb:
      fpng_channel_count = 3;
      break;
    default:
      fpng_channel_count = 3;
      try {
        converted_buffer =
            color::Convert(data, color, Color::kRgb, color::Standard::kBt709);
      } catch (const color::Error& convert_error) {
        throw error::TypedErrorBuilder(
                PngError::kColorConversion,
                "Failed to convert pixels to RGB before encoding")
            .Field("from", std::string(ColorName(color)))
            .Field("to", std::string(ColorName(Color::kRgb)))
            .Cause(error::ErrorBuilder("{}", convert_error.what()).Build())
            .Hint(
                "fpng only encodes RGB or RGBA; other formats are converted "
                "first.")
            .Build();
      }
      data = converted_buffer;
  }

  std::vector<std::uint8_t> output;
  if (!fpng::fpng_encode_image_to_memory(
          data.data(), static_cast<std::uint32_t>(width),
          static_cast<std::uint32_t>(height), fpng_channel_count, output)) {
    throw error::TypedErrorBuilder(PngError::kFpngEncode, "fpng encoding failed")
        .Field("width", width)
        .Field("height", height)
        .Field("channels", fpng_channel_count)
        .Hint(
            "This usually means the image exceeds fpng's size limits or the "
            "channel count is unsupported.")
        .Build();
  }
  if (output.empty()) {
    throw error::TypedErrorBuilder(PngError::kFpngEmptyOutput,
                                   "fpng produced empty output")
        .Field("width", width)
        .Field("height", height)
        .Build();
  }

  return output;
}

}  // namespace weqeqq::png
