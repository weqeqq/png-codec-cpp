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
target_link_libraries(your_target PngCodecStatic)  # or PngCodecShared
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
#include <png-codec-cpp/png.h>
#include <fstream>
#include <iostream>

int main() {
    try {
        // Read PNG file
        std::ifstream file("input.png", std::ios::binary);
        PngCodec::ByteContainer png_data((std::istreambuf_iterator<char>(file)),
                                         std::istreambuf_iterator<char>());

        // Decode PNG
        int width, height;
        auto pixels = PngCodec::Decode(png_data, height, width, PngCodec::Color::Rgba);

        std::cout << "Decoded " << width << "x" << height << " PNG image" << std::endl;

        // Encode back to PNG
        auto encoded = PngCodec::Encode(pixels, height, width, PngCodec::Color::Rgba);

        // Write to file
        std::ofstream output("output.png", std::ios::binary);
        output.write(reinterpret_cast<const char*>(encoded.data()), encoded.size());

    } catch (const PngCodec::Error& e) {
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
ByteContainer Decode(
    const ByteContainer &data,         // PNG file data
    int                 &row_count,    // Output: image height
    int                 &column_count, // Output: image width
    Color               color          // Desired output format
);

// Convenience overload (defaults to RGBA)
ByteContainer Decode(
    const ByteContainer &data,
    int                 &row_count,
    int                 &column_count
);
```

#### Encoding

```cpp
ByteContainer Encode(
    const ByteContainer &data,            // Pixel data
    int                  row_count,       // Image height
    int                  column_count,    // Image width
    Color                color            // Input pixel format
);
```

### Error Handling

The library uses exception-based error handling:

- `PngCodec::Error` - Base exception class
- `PngCodec::DecodeError` - Thrown during decoding failures
- `PngCodec::EncodeError` - Thrown during encoding failures

## Dependencies

- [Wuffs](https://github.com/google/wuffs) - Fast, safe PNG decoder
- [fpng](https://github.com/richgel999/fpng) - Fast PNG encoder

Dependencies are automatically fetched and built by CMake.

## License

This project is licensed under the MIT License.
