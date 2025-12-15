# TMCxx

[![CI](https://github.com/REDE-ARGE/TMCxx/actions/workflows/ci.yml/badge.svg)](https://github.com/REDE-ARGE/TMCxx/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> High-performance TMC5160 stepper driver implementation in modern C++. No macros, just strong types and templates.

## Features

- **Modern C++20** - Concepts, constexpr, strong types
- **Error Handling** - `tl::expected` based monadic error handling
- **Type-Safe Units** - RPM, Amperes, microsteps with compile-time validation
- **Zero-Cost Abstractions** - ~15KB binary size for full driver (Release build)
- **Cross-Platform** - Linux, Windows, macOS, Embedded (STM32/ESP32)
- **Header-Only** - Just include and use

## Quick Start

### Using Defaults

The easiest way to get started is using the default configuration which sets safe values for most standard NEMA17/23 motors.

```cpp
#include <tmcxx/tmc5160.hpp>

// Your SPI implementation
MySpi spi_device{};

// Initialize motor with default settings
tmcxx::TMC5160<MySpi>::Settings settings{}; // Default constructor sets safe defaults
tmcxx::TMC5160 motor{spi_device, settings};
using namespace tmcxx::units::literals;

// Apply default configuration (Safe Mode)
if (auto result{motor.apply_default_configuration()}; !result) 
{
    // Handle error: result.error()
    // e.g. ErrorCode::SPI_TRANSFER_FAILED
} 
else 
{
    // Ready to move
    motor.rotate(100_rpm);
}
```

### Using Builder (Advanced)

For precise control over current, velocity, and acceleration profiles:

```cpp
#include <tmcxx/builder/tmc_register_builder.hpp>
using namespace tmcxx::units::literals;

// ! The values ​​are random please set them according to the document and the engine you are using!
auto motor{tmcxx::helpers::builder::TMC5160Builder{spi_device}
    .clock_frequency(12.0_MHz)
    .sense_resistor(75.0_mOhm)
    .run_current(1.5_A)
    .hold_current(0.5_A)
    .toff(3U)
    .hysteresis(4U, 1U)
    .stealth_chop_enabled(true)
    .d_max(5000_pps2)
    .a_max(5000_pps2)
    .v_stop(10_rpm)
    .v_transition(100_rpm)
    .a_start(1000_pps2)
    .d_stop(5000_pps2)
    .power_down_delay(10U)
    .build()};

// apply_settings() returns helpers::result_t<void>
if (auto res{motor.apply_settings()}; !res) 
{
    // Handle configuration failure
}
```

## Installation

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    TMCxx
    GIT_REPOSITORY https://github.com/REDE-ARGE/TMCxx.git
    GIT_TAG main
)
FetchContent_MakeAvailable(TMCxx)

target_link_libraries(your_target PRIVATE tmc::xx)
```

### Manual

Copy the `include/tmcxx` directory to your project.

## Building

### Using CMake Presets (Recommended)

```bash
# List available presets
cmake --list-presets

# Development build (Debug + Ninja)
cmake --preset dev
cmake --build --preset dev
ctest --preset dev

# Release build
cmake --preset release
cmake --build --preset release
ctest --preset release

# Full workflow (configure + build + test)
cmake --workflow --preset dev
```

### Traditional CMake

```bash
cmake -B build -DTMCXX_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Requirements

- C++20 compatible compiler
- CMake 3.16+

## Platform Support

| Platform | Compilation | Runtime | Notes |
|----------|-------------|---------|-------|
| **STM32 HAL** | ✅ Passing | ✅ Passing | Verified on hardware, works flawlessly |
| **Linux** | ✅ Passing | ❓ Untested | Unit tests pass, SPI interaction via spidev possible |
| **ESP32** | ❓ Untested | ❓ Untested | Should work with appropriate cmake toolchain |
| **Other Embedded** | ❓ Untested | ❓ Untested | minimal C++20 support required |

> **Note:** Since this library is hardware-agnostic, it should work on any platform with C++20 support. Windows/macOS are supported for development and simulation (unit tests passing).

## Project Structure

```
TMCxx/
├── include/tmcxx/
│   ├── adapters/             # Platform-specific drivers (STM32)
│   ├── base/                 # Base types & concepts
│   ├── chips/                # Register definitions
│   ├── detail/               # Implementation details
│   ├── features/             # Converter, Communicator
│   ├── helpers/              # Units, constants, error codes
│   ├── vendor/               # Third-party (tl::expected)
│   └── tmc5160.hpp           # Main driver class
├── tests/                    # Unit tests
└── examples/                 # Example projects
```

## License

MIT License - see [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

**Note:** Parts of this documentation were generated with AI assistance. If you find any errors or inconsistencies, please [open an issue](https://github.com/REDE-ARGE/TMCxx/issues).
