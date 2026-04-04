#include <gtest/gtest.h>

#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <weqeqq/png.h>

namespace weqeqq::png {
namespace {

constexpr std::size_t kTestWidth = 4;
constexpr std::size_t kTestHeight = 3;

std::string ColorName(Color color) {
  switch (color) {
    case Color::kRgb:
      return "Rgb";
    case Color::kRgba:
      return "Rgba";
    case Color::kGrayscale:
      return "Grayscale";
    case Color::kCmyk:
      return "Cmyk";
    default:
      std::unreachable();
  }
}

std::size_t ExpectedByteSize(std::size_t width, std::size_t height, Color color) {
  return width * height * color::ChannelCount(color);
}

std::vector<std::uint8_t> MakeTestImage(std::size_t width, std::size_t height,
                                        Color color) {
  std::vector<std::uint8_t> data(ExpectedByteSize(width, height, color));

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      const auto pixel_index = y * width + x;
      const auto base = pixel_index * color::ChannelCount(color);

      const std::uint8_t a = static_cast<std::uint8_t>((x * 53 + y * 17) % 256);
      const std::uint8_t b = static_cast<std::uint8_t>((x * 29 + y * 71) % 256);
      const std::uint8_t c = static_cast<std::uint8_t>((x * 11 + y * 97) % 256);
      const std::uint8_t d = static_cast<std::uint8_t>((x * 83 + y * 19) % 256);

      switch (color) {
        case Color::kRgb:
          data[base + 0] = a;
          data[base + 1] = b;
          data[base + 2] = c;
          break;
        case Color::kRgba:
          data[base + 0] = a;
          data[base + 1] = b;
          data[base + 2] = c;
          data[base + 3] = static_cast<std::uint8_t>(128 + (d / 2));
          break;
        case Color::kGrayscale:
          data[base] = a;
          break;
        case Color::kCmyk:
          data[base + 0] = a;
          data[base + 1] = b;
          data[base + 2] = c;
          data[base + 3] = d;
          break;
        default:
          std::unreachable();
      }
    }
  }

  return data;
}

class PngColorFormatTest : public ::testing::TestWithParam<Color> {};

TEST_P(PngColorFormatTest, EncodeWorksForEveryInputColor) {
  const Color input_color = GetParam();
  const auto input = MakeTestImage(kTestWidth, kTestHeight, input_color);

  auto png_bytes = EncodeImage(input, kTestWidth, kTestHeight, input_color);

  EXPECT_FALSE(png_bytes.empty());
  EXPECT_TRUE(HasPngSignature(png_bytes));
}

TEST_P(PngColorFormatTest, DecodeToEveryRequestedColorWorks) {
  const Color output_color = GetParam();
  const auto rgb_input = MakeTestImage(kTestWidth, kTestHeight, Color::kRgb);
  const auto png_bytes = EncodeImage(rgb_input, kTestWidth, kTestHeight, Color::kRgb);

  auto decoded = DecodeImage(png_bytes, output_color);

  EXPECT_EQ(decoded.info.width, kTestWidth);
  EXPECT_EQ(decoded.info.height, kTestHeight);
  EXPECT_EQ(decoded.info.color, output_color);
  EXPECT_EQ(decoded.data.size(),
            ExpectedByteSize(kTestWidth, kTestHeight, output_color));
}

INSTANTIATE_TEST_SUITE_P(
    AllColorFormats, PngColorFormatTest,
    ::testing::Values(Color::kRgb, Color::kRgba, Color::kGrayscale, Color::kCmyk),
    [](const ::testing::TestParamInfo<Color>& info) { return ColorName(info.param); });

TEST(PngApiTest, HasPngSignatureWorks) {
  EXPECT_TRUE(HasPngSignature(Signature));
  EXPECT_FALSE(HasPngSignature(std::array<std::uint8_t, 4>{0x89, 0x50, 0x4E, 0x47}));

  auto bad = Signature;
  bad[1] = 0;
  EXPECT_FALSE(HasPngSignature(bad));
}

TEST(PngApiTest, DecodeRejectsBadSignature) {
  const std::array<std::uint8_t, 8> not_png = {0xFF, 0xD8, 0x4E, 0x47,
                                               0x0D, 0x0A, 0x1A, 0x0A};
  EXPECT_THROW((void)DecodeImage(not_png, Color::kRgb), ErrorInvalidPngSignature);
}

TEST(PngApiTest, EncodeRejectsInvalidInputParameters) {
  const std::array<std::uint8_t, 3> pixel = {0, 0, 0};
  EXPECT_THROW((void)EncodeImage(pixel, 0, 1, Color::kRgb), ErrorInvalidInputParameters);
}

TEST(PngApiTest, EncodeDecodeRoundtripSmoke) {
  const std::vector<std::uint8_t> rgb = {
      255, 0, 0,
      0, 255, 0,
      0, 0, 255,
      255, 255, 255,
  };

  auto png_bytes = EncodeImage(rgb, 2, 2, Color::kRgb);
  EXPECT_FALSE(png_bytes.empty());
  EXPECT_TRUE(HasPngSignature(png_bytes));

  auto decoded = DecodeImage(png_bytes, Color::kRgba);
  EXPECT_EQ(decoded.info.width, 2);
  EXPECT_EQ(decoded.info.height, 2);
  EXPECT_EQ(decoded.info.color, Color::kRgba);
  EXPECT_EQ(decoded.data.size(), 2u * 2u * 4u);
}

TEST(PngApiTest, ErrorMessageIncludesSourceLocationPrefix) {
  const std::array<std::uint8_t, 3> pixel = {0, 0, 0};
  try {
    (void)EncodeImage(pixel, 0, 1, Color::kRgb);
    FAIL() << "Expected ErrorInvalidInputParameters";
  } catch (const ErrorInvalidInputParameters& error) {
    const std::string_view what = error.what();
    EXPECT_NE(what.find('['), std::string_view::npos);
    EXPECT_NE(what.find(']'), std::string_view::npos);
    EXPECT_NE(what.find(':'), std::string_view::npos);
    EXPECT_NE(what.find("Invalid input parameters"), std::string_view::npos);
  }
}

}  // namespace
}  // namespace weqeqq::png
