# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2025-12-12

### Added

- Initial release
- TMC5160 driver support with motion control
- Strong type system for physical units (RPM, Amperes, microsteps, etc.)
- User-defined literals (`100_rpm`, `1.5_A`, `200_steps`)
- Compile-time register validation
- Unit converter for velocity, acceleration, and current
- Cross-platform CI (Linux, Windows, macOS)
- CMake package support (`find_package`, `FetchContent`, `pkg-config`)

### Features

- `TMC5160` main driver class
- `TMC5160Motion` for velocity and position control
- `Converter` for unit conversions
- `Quantity<Tag, T>` strong type wrapper
- Full register map for TMC5160

### Supported Compilers

- GCC 13+
- Clang 18+
- MSVC 2019+
