module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module wqpng.tests.png;

import weqeqq.png;
import weqeqq.test;

namespace weqeqq::png {
namespace {

using namespace weqeqq::test;

constexpr std::size_t kWidth = 4;
constexpr std::size_t kHeight = 3;

std::size_t ByteSize(std::size_t width, std::size_t height, Color color) {
  return width * height * color::ChannelCount(color);
}

std::vector<std::uint8_t> MakeImage(std::size_t width, std::size_t height,
                                    Color color) {
  std::vector<std::uint8_t> data(ByteSize(width, height, color));
  const std::size_t channels = color::ChannelCount(color);
  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      const std::size_t base = (y * width + x) * channels;
      for (std::size_t channel = 0; channel < channels; ++channel) {
        data[base + channel] =
            static_cast<std::uint8_t>((x * 53 + y * 17 + channel * 29) % 256);
      }
    }
  }
  return data;
}

const Suite kSignature("signature", [] {
  Test("accepts the canonical signature", [] {
    Expect(HasPngSignature(Signature), IsTrue());
  });

  Test("rejects a truncated buffer", [] {
    const std::array<std::uint8_t, 4> prefix = {0x89, 0x50, 0x4E, 0x47};
    Expect(HasPngSignature(prefix), IsFalse());
  });

  Test("rejects a corrupted signature", [] {
    auto bytes = Signature;
    bytes[1] = 0;
    Expect(HasPngSignature(bytes), IsFalse());
  });

  Test("accepts a matching span", [] {
    const std::span<const std::uint8_t> span(Signature);
    Expect(HasPngSignature(span), IsTrue());
  });
});

const Suite kEncode("encode", [] {
  Test("produces a signed payload for rgb input", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgb);
    Expect(bytes.empty(), IsFalse());
    Expect(HasPngSignature(bytes), IsTrue());
  });

  Test("produces a signed payload for rgba input", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgba);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgba);
    Expect(bytes.empty(), IsFalse());
    Expect(HasPngSignature(bytes), IsTrue());
  });

  Test("accepts grayscale input by converting to rgb", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kGrayscale);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kGrayscale);
    Expect(bytes.empty(), IsFalse());
    Expect(HasPngSignature(bytes), IsTrue());
  });

  Test("rejects empty input", [] {
    const std::vector<std::uint8_t> empty;
    Expect([&] { EncodeImage(empty, kWidth, kHeight, Color::kRgb); },
           Throws<error::TypedError<PngError>>());
  });

  Test("rejects zero dimensions", [] {
    const std::array<std::uint8_t, 3> pixel = {0, 0, 0};
    Expect([&] { EncodeImage(pixel, 0, 1, Color::kRgb); },
           Throws<error::TypedError<PngError>>());
  });

  Test("reports invalid parameters through the typed code", [] {
    const std::array<std::uint8_t, 3> pixel = {0, 0, 0};
    try {
      EncodeImage(pixel, 0, 1, Color::kRgb);
      Expect(false) << "expected EncodeImage to throw";
    } catch (const error::TypedError<PngError>& error) {
      Expect(error.TypedCode() == PngError::kInvalidInputParameters, IsTrue());
    }
  });

  Test("rejects a buffer that does not match the dimensions", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const std::span<const std::uint8_t> truncated(image.data(),
                                                  image.size() - 3);
    try {
      EncodeImage(truncated, kWidth, kHeight, Color::kRgb);
      Expect(false) << "expected EncodeImage to throw";
    } catch (const error::TypedError<PngError>& error) {
      Expect(error.TypedCode() == PngError::kInvalidInputParameters, IsTrue());
    }
  });
});

const Suite kDecode("decode", [] {
  Test("rejects empty input", [] {
    const std::vector<std::uint8_t> empty;
    Expect([&] { DecodeImage(empty, Color::kRgb); },
           Throws<error::TypedError<PngError>>());
  });

  Test("rejects a bad signature", [] {
    const std::array<std::uint8_t, 8> not_png = {0xFF, 0xD8, 0x4E, 0x47,
                                                 0x0D, 0x0A, 0x1A, 0x0A};
    Expect([&] { DecodeImage(not_png, Color::kRgb); },
           Throws<error::TypedError<PngError>>());
  });

  Test("reports a bad signature through the typed code", [] {
    const std::array<std::uint8_t, 8> not_png = {0xFF, 0xD8, 0x4E, 0x47,
                                                 0x0D, 0x0A, 0x1A, 0x0A};
    try {
      DecodeImage(not_png, Color::kRgb);
      Expect(false) << "expected DecodeImage to throw";
    } catch (const error::TypedError<PngError>& error) {
      Expect(error.TypedCode() == PngError::kInvalidSignature, IsTrue());
    }
  });
});

const Suite kRoundtrip("roundtrip", [] {
  Test("preserves dimensions and size for rgb output", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgb);
    const auto decoded = DecodeImage(bytes, Color::kRgb);
    Expect(decoded.info.width, Eq(kWidth));
    Expect(decoded.info.height, Eq(kHeight));
    Expect(decoded.info.color == Color::kRgb, IsTrue());
    Expect(decoded.data.size(), Eq(ByteSize(kWidth, kHeight, Color::kRgb)));
  });

  Test("recovers identical rgb pixels", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgb);
    const auto decoded = DecodeImage(bytes, Color::kRgb);
    Expect(decoded.data == image, IsTrue());
  });

  Test("decodes rgb input to rgba output", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgb);
    const auto decoded = DecodeImage(bytes, Color::kRgba);
    Expect(decoded.info.color == Color::kRgba, IsTrue());
    Expect(decoded.data.size(), Eq(ByteSize(kWidth, kHeight, Color::kRgba)));
  });

  Test("decodes rgb input to grayscale output", [] {
    const auto image = MakeImage(kWidth, kHeight, Color::kRgb);
    const auto bytes = EncodeImage(image, kWidth, kHeight, Color::kRgb);
    const auto decoded = DecodeImage(bytes, Color::kGrayscale);
    Expect(decoded.info.color == Color::kGrayscale, IsTrue());
    Expect(decoded.data.size(),
           Eq(ByteSize(kWidth, kHeight, Color::kGrayscale)));
  });
});

}  // namespace
}  // namespace weqeqq::png
