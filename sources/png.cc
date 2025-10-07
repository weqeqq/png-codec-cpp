// png.cc
#include <fpng.h>
#include <png-codec-cpp/png.h>
#include <wuffs.h>

#include <cstring>
#include <memory>
#include <vector>

namespace PngCodec {

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

std::vector<std::uint8_t> Decode(const std::uint8_t *data, std::size_t length,
                                 std::size_t &row_count,
                                 std::size_t &column_count, Color color) {
  if (!data || length == 0) {
    throw DecodeError("Invalid input data");
  }

  std::unique_ptr<wuffs_png__decoder, decltype(&free)> decoder(
      wuffs_png__decoder__alloc(), &free);
  if (!decoder) {
    throw DecodeError("Failed to allocate PNG decoder");
  }

  wuffs_base__status status = wuffs_png__decoder__initialize(
      decoder.get(), sizeof__wuffs_png__decoder(), WUFFS_VERSION, 0);
  if (status.repr) {
    throw DecodeError("Failed to initialize PNG decoder: " +
                      std::string(status.repr));
  }

  wuffs_base__io_buffer source = wuffs_base__ptr_u8__reader(
      const_cast<std::uint8_t *>(data), length, true);

  wuffs_base__image_config config;
  status =
      wuffs_png__decoder__decode_image_config(decoder.get(), &config, &source);
  if (status.repr) {
    throw DecodeError("Failed to decode PNG image config: " +
                      std::string(status.repr));
  }

  row_count = wuffs_base__pixel_config__height(&config.pixcfg);
  column_count = wuffs_base__pixel_config__width(&config.pixcfg);

  if (row_count == 0 || column_count == 0) {
    throw DecodeError("Invalid image dimensions");
  }

  wuffs_base__pixel_config__set(
      &config.pixcfg, static_cast<std::underlying_type_t<Color>>(color),
      WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, column_count, row_count);

  auto workbuf_len = wuffs_png__decoder__workbuf_len(decoder.get()).max_incl;
  std::unique_ptr<std::uint8_t[]> workbuf(new std::uint8_t[workbuf_len]);
  wuffs_base__slice_u8 workbuf_slice =
      wuffs_base__make_slice_u8(workbuf.get(), workbuf_len);

  auto pixel_format = wuffs_base__make_pixel_format(
      static_cast<std::underlying_type_t<Color>>(color));
  std::size_t total_bytes =
      row_count * column_count * pixel_format.bits_per_pixel() / 8;

  std::vector<std::uint8_t> buffer(total_bytes);
  auto pixbuf_slice = wuffs_base__make_slice_u8(buffer.data(), buffer.size());

  wuffs_base__pixel_buffer pixbuf;
  status = wuffs_base__pixel_buffer__set_from_slice(&pixbuf, &config.pixcfg,
                                                    pixbuf_slice);
  if (status.repr) {
    throw DecodeError("Failed to set pixel buffer: " +
                      std::string(status.repr));
  }

  status = wuffs_png__decoder__decode_frame(decoder.get(), &pixbuf, &source,
                                            WUFFS_BASE__PIXEL_BLEND__SRC,
                                            workbuf_slice, nullptr);
  if (status.repr) {
    throw DecodeError("Failed to decode PNG frame: " +
                      std::string(status.repr));
  }
  return buffer;
}

std::vector<std::uint8_t> Encode(const std::uint8_t *data,
                                 std::size_t row_count,
                                 std::size_t column_count, Color color) {
  if (!data || row_count == 0 || column_count == 0) {
    throw EncodeError("Invalid input parameters");
  }
  static FpngInit fpng_init;

  std::uint32_t channel_count = 0;
  switch (color) {
    case Color::Rgba:
      channel_count = 4;
      break;
    case Color::Rgb:
      channel_count = 3;
      break;
    default:
      throw EncodeError("Unsupported color format (only RGB/RGBA supported)");
  }

  std::vector<std::uint8_t> output;
  if (!fpng::fpng_encode_image_to_memory(data, column_count, row_count,
                                         channel_count, output)) {
    throw EncodeError("fpng encoding failed");
  }
  if (output.empty()) {
    throw EncodeError("fpng produced empty output");
  }
  return output;
}
}  // namespace PngCodec
