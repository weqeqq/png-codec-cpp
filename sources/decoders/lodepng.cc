
#ifdef WQPNG_USE_LODEPNG_FOR_DECODING

#include <lodepng.h>
#include <weqeqq/png.h>

#include <utility>
#include <vector>

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

DecodedImage DecodeImage(std::span<const std::uint8_t> data, Color color) {
  if (data.empty()) {
    throw ErrorInvalidInputData();
  }
  if (data.size() < Signature.size() || !HasPngSignature(data)) {
    throw ErrorInvalidPngSignature();
  }

  auto lct = LodePngColorTypeIsSupported(color) ? ColorToLodePngColorType(color)
                                                : LCT_RGB;
  DecodedImage output;
  unsigned w;
  unsigned h;
  unsigned error =
      lodepng::decode(output.data, w, h, data.data(), data.size(), lct);
  if (error) {
    throw ErrorLodepngDecodingFailed(error, lodepng_error_text(error));
  }
  if (w == 0 || h == 0) {
    throw ErrorInvalidImageDimensions(w, h);
  }
  output.info.width = w;
  output.info.height = h;
  output.info.color = color;

  if (!LodePngColorTypeIsSupported(color)) {
    output.data = color::Convert(output.data, Color::kRgb, color,
                                 color::Standard::kBt709);
  }
  return output;
}

}  // namespace weqeqq::png

#endif
