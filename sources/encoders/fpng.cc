
#ifdef WQPNG_USE_FPNG_FOR_ENCODING

#include <fpng.h>
#include <weqeqq/png.h>

#include <cstring>
#include <span>
#include <vector>

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
  if (data.empty() || height == 0 || width == 0) {
    throw ErrorInvalidInputParameters();
  }
  static FpngInit fpng_init;
  std::vector<std::uint8_t> converted_buffer;

  std::uint32_t channel_count = 0;
  switch (color) {
    case Color::kRgba:
      channel_count = 4;
      break;
    case Color::kRgb:
      channel_count = 3;
      break;
    default:
      channel_count = 3;
      converted_buffer =
          color::Convert(data, color, Color::kRgb, color::Standard::kBt709);
      data = converted_buffer;
  }
  std::vector<std::uint8_t> output;
  if (!fpng::fpng_encode_image_to_memory(
          data.data(), static_cast<uint32_t>(width),
          static_cast<uint32_t>(height), channel_count, output)) {
    throw ErrorFpngEncodingFailed();
  }
  if (output.empty()) {
    throw ErrorFpngEmptyOutput();
  }

  return output;
}
}  // namespace weqeqq::png

#endif
