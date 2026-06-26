module;

#include <string>
#include <system_error>
#include <type_traits>

export module weqeqq.png:error;

export import weqeqq.error;

export namespace weqeqq::png {

// Error codes for every failure mode the PNG codec can report. They map onto
// std::error_code so they can travel through weqeqq::error::TypedError and be
// inspected by callers via Error::Code() / TypedError::TypedCode().
enum class PngError {
  kInvalidInputData = 1,
  kInvalidSignature,
  kInvalidInputParameters,
  kInvalidDimensions,
  kWuffsAlloc,
  kWuffsInit,
  kWuffsConfig,
  kWuffsPixelBuffer,
  kWuffsFrame,
  kFpngEncode,
  kFpngEmptyOutput,
  kColorConversion,
};

class PngErrorCategory : public std::error_category {
 public:
  [[nodiscard]] const char* name() const noexcept override {
    return "weqeqq.png";
  }

  [[nodiscard]] std::string message(int value) const override {
    switch (static_cast<PngError>(value)) {
      case PngError::kInvalidInputData:
        return "Invalid input data";
      case PngError::kInvalidSignature:
        return "Invalid PNG signature";
      case PngError::kInvalidInputParameters:
        return "Invalid input parameters";
      case PngError::kInvalidDimensions:
        return "Invalid image dimensions";
      case PngError::kWuffsAlloc:
        return "Failed to allocate PNG decoder";
      case PngError::kWuffsInit:
        return "Failed to initialize PNG decoder";
      case PngError::kWuffsConfig:
        return "Failed to decode PNG image config";
      case PngError::kWuffsPixelBuffer:
        return "Failed to set pixel buffer";
      case PngError::kWuffsFrame:
        return "Failed to decode PNG frame";
      case PngError::kFpngEncode:
        return "fpng encoding failed";
      case PngError::kFpngEmptyOutput:
        return "fpng produced empty output";
      case PngError::kColorConversion:
        return "Color conversion failed";
    }
    return "Unknown weqeqq.png error";
  }
};

[[nodiscard]] inline const std::error_category& png_category() noexcept {
  static const PngErrorCategory category;
  return category;
}

[[nodiscard]] inline std::error_code make_error_code(PngError error) noexcept {
  return {static_cast<int>(error), png_category()};
}

}  // namespace weqeqq::png

namespace std {
template <>
struct is_error_code_enum<weqeqq::png::PngError> : true_type {};
}  // namespace std
