
#include <fpng.h>
#include <png-codec-cpp/png.h>
#include <wuffs.h>

namespace PngCodec {

/* clang-format off */
ByteContainer Decode(
  const ByteContainer &data,
  int                 &row_count,
  int                 &column_count,
  Color                color
) {
  std::unique_ptr<wuffs_png__decoder, decltype(&free)> decoder(
    wuffs_png__decoder__alloc(),
    &free
  );
  if (!decoder) {
    throw DecodeError("Failed to allocate PNG decoder");
  }
  wuffs_base__image_config config;
  wuffs_base__io_buffer source = wuffs_base__ptr_u8__reader(
    const_cast<std::uint8_t *>(data.data()),
    data.size(),
    true
  );
  wuffs_base__status status = wuffs_png__decoder__decode_image_config(
    decoder.get(),
    &config,
    &source
  );
  if (status.repr) {
    throw DecodeError("Failed to decode PNG image config: " + std::string(status.repr));
  }
  row_count = wuffs_base__pixel_config__height(
    &config.pixcfg
  );
  column_count = wuffs_base__pixel_config__width(
    &config.pixcfg
  );
  wuffs_base__pixel_config__set(
    &config.pixcfg,
    static_cast<std::underlying_type_t<Color>>(color),
    WUFFS_BASE__PIXEL_SUBSAMPLING__NONE,
    column_count,
    row_count
  );
  auto workbuf_len = wuffs_png__decoder__workbuf_len(decoder.get()).max_incl;
  std::unique_ptr<std::uint8_t[]> workbuf(new std::uint8_t[workbuf_len]);

  wuffs_base__slice_u8 workbuf_slice = wuffs_base__make_slice_u8(
    workbuf.get(),
    workbuf_len
  );
  auto total =
    row_count    *
    column_count *
    wuffs_base__make_pixel_format(static_cast<std::underlying_type_t<Color>>(color)).bits_per_pixel() / 8;

  ByteContainer buffer(total);
  auto pixbuf_slice = wuffs_base__make_slice_u8(buffer.data(), buffer.size());

  wuffs_base__pixel_buffer pixbuf;
  status = wuffs_base__pixel_buffer__set_from_slice(
    &pixbuf,
    &config.pixcfg,
    pixbuf_slice
  );
  if (status.repr) {
    throw DecodeError("Failed to set pixel buffer: " + std::string(status.repr));
  }
  status = wuffs_png__decoder__decode_frame(
    decoder.get(),
    &pixbuf,
    &source,
    WUFFS_BASE__PIXEL_BLEND__SRC,
    workbuf_slice,
    nullptr
  );
  if (status.repr) {
    throw DecodeError("Failed to decode PNG frame: " + std::string(status.repr));
  }
  return buffer;
}
ByteContainer Encode(
  const ByteContainer &data,
  int                  row_count,
  int                  column_count,
  Color                color
) {
  auto channel_count = 0;
  switch (color) {
    case Color::Rgba : channel_count = 4; break;
    case Color::Rgb  : channel_count = 3; break;
    default: {
      throw EncodeError("Unsupported color");
    }
  }
  ByteContainer output;
  fpng::fpng_encode_image_to_memory(
    data.data(),
    column_count,
    row_count,
    channel_count,
    output
  );
  return output;
}
/* clang-format on */

}  // namespace PngCodec
