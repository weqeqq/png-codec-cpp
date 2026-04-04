
#ifdef WQPNG_USE_WUFFS_FOR_DECODING

#include <weqeqq/png.h>

#include <cstring>
#include <memory>
#include <span>
#include <utility>
#include <vector>
#include <wuffs.h>

namespace weqeqq::png {

bool WuffsColorIsSupported(Color color) {
  switch (color) {
    case Color::kRgb:
      return true;
    case Color::kRgba:
      return true;
    case Color::kGrayscale:
      return false;
    case Color::kCmyk:
      return false;
    default:
      std::unreachable();
  }
}

std::uint32_t ColorToWuffsPixfmt(Color color) {
  switch (color) {
    case Color::kRgb:
      return WUFFS_BASE__PIXEL_FORMAT__RGB;
    case Color::kRgba:
      return WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL;
    default:
      std::unreachable();
  }
}

DecodedImage DecodeImage(std::span<const std::uint8_t> data, Color color) {
  if (data.empty()) {
    throw ErrorInvalidInputData();
  }
  if (data.size() < Signature.size()) {
    throw ErrorInvalidPngSignature();
  }
  if (!HasPngSignature(data)) {
    throw ErrorInvalidPngSignature();
  }

  std::unique_ptr<wuffs_png__decoder, decltype(&free)> decoder(
      wuffs_png__decoder__alloc(), &free);
  if (!decoder) {
    throw ErrorWuffsPngDecoderAllocationFailed();
  }

  wuffs_base__status status = wuffs_png__decoder__initialize(
      decoder.get(), sizeof__wuffs_png__decoder(), WUFFS_VERSION, 0);
  if (status.repr) {
    throw ErrorWuffsPngDecoderInitializationFailed(status.repr);
  }

  wuffs_base__io_buffer source = wuffs_base__ptr_u8__reader(
      const_cast<std::uint8_t*>(data.data()), data.size(), true);

  wuffs_base__image_config config;
  status =
      wuffs_png__decoder__decode_image_config(decoder.get(), &config, &source);
  if (status.repr) {
    throw ErrorWuffsPngImageConfigDecodeFailed(status.repr);
  }

  auto height = wuffs_base__pixel_config__height(&config.pixcfg);
  auto width = wuffs_base__pixel_config__width(&config.pixcfg);

  if (height == 0 || width == 0) {
    throw ErrorInvalidImageDimensions(width, height);
  }

  std::uint32_t pixfmt = WuffsColorIsSupported(color)
                             ? ColorToWuffsPixfmt(color)
                             : ColorToWuffsPixfmt(Color::kRgb);

  wuffs_base__pixel_config__set(&config.pixcfg, pixfmt,
                                WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, width,
                                height);

  auto workbuf_len = wuffs_png__decoder__workbuf_len(decoder.get()).max_incl;
  std::unique_ptr<std::uint8_t[]> workbuf(new std::uint8_t[workbuf_len]);
  wuffs_base__slice_u8 workbuf_slice =
      wuffs_base__make_slice_u8(workbuf.get(), workbuf_len);

  auto pixel_format = wuffs_base__make_pixel_format(pixfmt);
  std::size_t total_bytes = height * width * pixel_format.bits_per_pixel() / 8;

  std::vector<std::uint8_t> buffer(total_bytes);
  auto pixbuf_slice = wuffs_base__make_slice_u8(buffer.data(), buffer.size());

  wuffs_base__pixel_buffer pixbuf;
  status = wuffs_base__pixel_buffer__set_from_slice(&pixbuf, &config.pixcfg,
                                                    pixbuf_slice);
  if (status.repr) {
    throw ErrorWuffsPixelBufferSetupFailed(status.repr);
  }

  status = wuffs_png__decoder__decode_frame(decoder.get(), &pixbuf, &source,
                                            WUFFS_BASE__PIXEL_BLEND__SRC,
                                            workbuf_slice, nullptr);
  if (status.repr) {
    throw ErrorWuffsPngFrameDecodeFailed(status.repr);
  }

  if (!WuffsColorIsSupported(color)) {
    buffer =
        color::Convert(buffer, Color::kRgb, color, color::Standard::kBt709);
  }
  return DecodedImage{
      .data = std::move(buffer),
      .info = ImageInfo{.width = width, .height = height, .color = color}};
}

}  // namespace weqeqq::png

#endif
