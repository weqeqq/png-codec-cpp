module;

#include <wuffs.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

module weqeqq.png:decoder;

import :types;
import :error;
import weqeqq.color;

namespace weqeqq::png {
namespace {

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

error::Error WuffsCause(const char* repr) {
  return error::ErrorBuilder("wuffs reported: {}", repr).Build();
}

}  // namespace

DecodedImage DecodeImage(std::span<const std::uint8_t> data, Color color) {
  if (data.empty()) {
    throw error::TypedErrorBuilder(PngError::kInvalidInputData,
                                   "Cannot decode an empty buffer")
        .Field("requested_color", std::string(ColorName(color)))
        .Hint("Pass the full PNG file contents, including the 8-byte signature.")
        .Build();
  }
  if (data.size() < Signature.size() || !HasPngSignature(data)) {
    throw error::TypedErrorBuilder(
            PngError::kInvalidSignature,
            "Buffer does not start with the PNG signature")
        .Field("buffer_size", data.size())
        .Details(
            "A PNG stream must begin with the 8 bytes "
            "89 50 4E 47 0D 0A 1A 0A.")
        .Hint(
            "Verify the data is a PNG file and was not truncated or wrapped in "
            "another container.")
        .Build();
  }

  std::unique_ptr<wuffs_png__decoder, decltype(&free)> decoder(
      wuffs_png__decoder__alloc(), &free);
  if (!decoder) {
    throw error::TypedErrorBuilder(PngError::kWuffsAlloc,
                                   "Failed to allocate the PNG decoder")
        .Details(
            "wuffs_png__decoder__alloc returned null, which indicates the "
            "process is out of memory.")
        .Build();
  }

  wuffs_base__status status = wuffs_png__decoder__initialize(
      decoder.get(), sizeof__wuffs_png__decoder(), WUFFS_VERSION, 0);
  if (status.repr) {
    throw error::TypedErrorBuilder(PngError::kWuffsInit,
                                   "Failed to initialize the PNG decoder")
        .Cause(WuffsCause(status.repr))
        .Build();
  }

  wuffs_base__io_buffer source = wuffs_base__ptr_u8__reader(
      const_cast<std::uint8_t*>(data.data()), data.size(), true);

  wuffs_base__image_config config;
  status =
      wuffs_png__decoder__decode_image_config(decoder.get(), &config, &source);
  if (status.repr) {
    throw error::TypedErrorBuilder(PngError::kWuffsConfig,
                                   "Failed to read the PNG header")
        .Field("buffer_size", data.size())
        .Cause(WuffsCause(status.repr))
        .Hint("The PNG header is malformed or the stream is not a PNG.")
        .Build();
  }

  auto height = wuffs_base__pixel_config__height(&config.pixcfg);
  auto width = wuffs_base__pixel_config__width(&config.pixcfg);

  if (height == 0 || width == 0) {
    throw error::TypedErrorBuilder(PngError::kInvalidDimensions,
                                   "PNG reports zero-sized dimensions")
        .Field("width", width)
        .Field("height", height)
        .Hint("The file declares a width or height of zero and holds no pixels.")
        .Build();
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
    throw error::TypedErrorBuilder(PngError::kWuffsPixelBuffer,
                                   "Failed to set up the pixel buffer")
        .Field("width", width)
        .Field("height", height)
        .Field("requested_color", std::string(ColorName(color)))
        .Cause(WuffsCause(status.repr))
        .Build();
  }

  status = wuffs_png__decoder__decode_frame(decoder.get(), &pixbuf, &source,
                                            WUFFS_BASE__PIXEL_BLEND__SRC,
                                            workbuf_slice, nullptr);
  if (status.repr) {
    throw error::TypedErrorBuilder(PngError::kWuffsFrame,
                                   "Failed to decode the PNG image data")
        .Field("width", width)
        .Field("height", height)
        .Cause(WuffsCause(status.repr))
        .Hint("The compressed image data is corrupt or truncated.")
        .Build();
  }

  if (!WuffsColorIsSupported(color)) {
    try {
      buffer =
          color::Convert(buffer, Color::kRgb, color, color::Standard::kBt709);
    } catch (const color::Error& convert_error) {
      throw error::TypedErrorBuilder(
              PngError::kColorConversion,
              "Failed to convert decoded pixels to the requested color")
          .Field("from", std::string(ColorName(Color::kRgb)))
          .Field("to", std::string(ColorName(color)))
          .Cause(error::ErrorBuilder("{}", convert_error.what()).Build())
          .Build();
    }
  }
  return DecodedImage{
      .data = std::move(buffer),
      .info = ImageInfo{.width = width, .height = height, .color = color}};
}

}  // namespace weqeqq::png
