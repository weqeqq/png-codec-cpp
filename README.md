# PNG Codec C++

A high-performance C++ library for encoding and decoding PNG images, built on top of Google's Wuffs and fpng libraries.

## Features

- Fast PNG decoding using Google's Wuffs library
- Fast PNG encoding using fpng library
- Support for multiple color formats (RGB, RGBA, BGR, BGRA, RGBX, BGRX, CMYK)
- Cross-platform support (Windows, Linux, macOS)
- Both shared and static library builds
- Modern C++17 interface
- Exception-based error handling

## Installation

### CMake

This project uses CMake and automatically fetches its dependencies. Simply include it in your CMake project:

```cmake
add_subdirectory(png-codec-cpp)
target_link_libraries(your_target Png::Png)  # Shared or static based on BUILD_SHARED_LIBS
```

### Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Basic Example

```cpp
#include <wqpng/png.h>
#include <fstream>
#include <iostream>

int main() {
    try {
        // Read PNG file
        std::ifstream file("input.png", std::ios::binary);
        std::vector<std::uint8_t> png_data((std::istreambuf_iterator<char>(file)),
                                         std::istreambuf_iterator<char>());

        // Decode PNG
        std::size_t width, height;
        auto pixels = Png::Decode(png_data, height, width, Png::Color::Rgba);

        std::cout << "Decoded " << width << "x" << height << " PNG image" << std::endl;

        // Encode back to PNG
        auto encoded = Png::Encode(pixels, height, width, Png::Color::Rgba);

        // Write to file
        std::ofstream output("output.png", std::ios::binary);
        output.write(reinterpret_cast<const char*>(encoded.data()), encoded.size());

    } catch (const Png::Error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### Color Formats

The library supports various color formats:

- `Color::Rgb` - RGB (24-bit)
- `Color::Rgba` - RGBA (32-bit with alpha)
- `Color::Rgbx` - RGBX (32-bit, X ignored)
- `Color::Bgr` - BGR (24-bit)
- `Color::Bgra` - BGRA (32-bit with alpha)
- `Color::Bgrx` - BGRX (32-bit, X ignored)
- `Color::Cmyk` - CMYK (32-bit)

### API Reference

#### Decoding

```cpp
std::vector<std::uint8_t> Decode(
    const std::vector<std::uint8_t> &data,  // PNG file data
    std::size_t                     &row_count,    // Output: image height
    std::size_t                     &column_count, // Output: image width
    Color                           color          // Desired output format
);

// Convenience overload (defaults to RGBA)
std::vector<std::uint8_t> Decode(
    const std::vector<std::uint8_t> &data,
    std::size_t                     &row_count,
    std::size_t                     &column_count
);
```

#### Encoding

```cpp
std::vector<std::uint8_t> Encode(
    const std::vector<std::uint8_t> &data,            // Pixel data
    std::size_t                      row_count,       // Image height
    std::size_t                      column_count,    // Image width
    Color                            color            // Input pixel format
);
```

### Error Handling

The library uses exception-based error handling:

- `Png::Error` - Base exception class
- `Png::DecodeError` - Thrown during decoding failures
- `Png::EncodeError` - Thrown during encoding failures

## Dependencies

- [Wuffs](https://github.com/google/wuffs) - Fast, safe PNG decoder
- [fpng](https://github.com/richgel999/fpng) - Fast PNG encoder

Dependencies are automatically fetched and built by CMake.

## License

This project is licensed under the MIT License.
