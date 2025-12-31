# Contributing to ESP32-BinForge

Thank you for your interest in contributing to ESP32-BinForge! Everyone is welcome to contribute, whether it’s code, documentation, examples, or ideas. Your contributions help make the tool more powerful, user-friendly, and useful for post-build modification and analysis of ESP32 firmware binaries.

## How to Contribute

You can contribute in several ways:

- Reporting issues or bugs
- Suggesting features or improvements
- Submitting pull requests for fixes, enhancements, or new functionality
- Providing build instructions or precompiled binaries for additional platforms (Linux, macOS)

## All contributions should preserve the Original Workflow
Any contributions that introduce alternate data injection methods or workflows must be fully optional. The original BinForge workflow, using the magic blob for post-build injection, must remain intact and fully functional. Primarily to support cases where the firmware is already finalized, redistributed, built elsewhere, or closed-source, and the only reliable input available is the binary itself.

## Limitations and Compatibility

Please be aware of the current limitations of ESP32-BinForge:

- Designed primarily for ESP32 series MCUs.
- Tested toolchains:
  - ESP-IDF + PlatformIO
  - Arduino IDE
- Binary modification relies on unencrypted firmware images

When submitting contributions, please clearly indicate which MCUs and toolchains your changes have been tested with.

## Precompiled Binaries

- Precompiled binaries for Linux and macOS are **not provided** at this time.
- Contributions that provide cross-platform builds, instructions, or CI scripts for Linux/macOS are welcome.
- Windows precompiled binaries are available as part of the repository.

## Code of Conduct

By contributing to ESP32-BinForge, you agree to follow the [MIT License](LICENSE) and maintain a respectful and constructive environment.

## Getting Started

1. Fork the repository
2. Make your addition 
3. Test thoroughly on supported platforms
4. Document any limitations(MCU specific additions for example)
5. Submit a pull request with a detailed description of your changes

Feel free to reach out if you have specific questions.


I welcome contributions of any size. Even small fixes, bug reports, or suggestions help improve ESP32-BinForge for everyone.
