
#ifdef WQPNG_USE_LODEPNG_FOR_ENCODING

#include <lodepng.h>
#include <weqeqq/png.h>

namespace weqeqq::png {
namespace {

bool LodePngColorTypeIsSupported(Color color) {
  switch (color) {
    case Color::kRgb:
      return true;
    case Color::kRgba:
      return true;
    case Color::kGrayscale:
      return true;
    case Color::kCmyk:
      return false;
    default:
      std::unreachable();
  }
}

LodePNGColorType ColorToLodePngColorType(Color color) {
  switch (color) {
    case Color::kRgb:
      return LCT_RGB;
    case Color::kRgba:
      return LCT_RGBA;
    case Color::kGrayscale:
      return LCT_GREY;
    default:
      std::unreachable();
  }
}
}  // namespace

std::vector<std::uint8_t> EncodeImage(std::span<const std::uint8_t> data,
                                      std::size_t width, std::size_t height,
                                      Color color) {
  if (data.empty() || height == 0 || width == 0) {
    throw ErrorInvalidInputParameters();
  }
  std::vector<std::uint8_t> converted_buffer;

  LodePNGColorType lct;
  if (LodePngColorTypeIsSupported(color)) {
    lct = ColorToLodePngColorType(color);
  } else {
    lct = LCT_RGB;
    converted_buffer =
        color::Convert(data, color, Color::kRgb, color::Standard::kBt709);
    data = converted_buffer;
  }
  std::vector<std::uint8_t> output;
  unsigned error =
      lodepng::encode(output, data.data(), static_cast<unsigned>(width),
                      static_cast<unsigned>(height), lct);
  if (error) {
    throw ErrorLodepngEncodingFailed(error);
  }
  if (output.empty()) {
    throw ErrorLodepngEmptyOutput();
  }

  return output;
}

}  // namespace weqeqq::png

#endif
