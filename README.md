# ESP32-BinForge

**ESP32-BinForge** is a small utility for modifying and revalidating ESP32 firmware .bin files. It supports injecting custom data into firmware images and regenerating CRCs, allowing binaries to remain flashable after intentional or accidental modification.

The tool is designed for workflows where post-build customization is needed without recompiling firmware, such as embedding assets, configuration blobs, or replacing placeholder data.

The tool provides a reserved embedded region. The interpretation and use of that region is entirely user-defined.

Data regions are defined by a randomly generated magic byte sequence. This sequence is intentionally non‑compressible and prevents toolchain or linker optimizations from collapsing or deduplicating the region during compilation.

Until user data is injected, the contents of this region should be considered undefined.

---

## Current Features

- **Data injection**
  - Injects user-supplied data into predefined regions of a firmware image
  - Automatically regenerates the CRC after injection

- **CRC-only mode**
  - Regenerates the CRC for binaries modified by external tools

- **SHA-256 handling**
  - Currently disables the SHA-256 check (required for modified images)
  - SHA-256 regeneration is planned for a future release

---

## What It Does *Not* Do (Yet)

- Does **not** currently detect or report structural errors in firmware images
- Does **not** regenerate SHA-256 hashes
- Does **not** validate injected data correctness beyond checksum updates

---

## Use Cases

- Post-compilation firmware customization  
- Asset or data blob injection into ESP32 binaries  
- Repairing CRCs after manual or automated binary edits  
- Development and tooling workflows for ESP32 firmware experimentation

---

## Scope and Compatibility

- Based on ESP32 firmware image specifications  
- Intended for ESP32-class devices  
  (Compatibility with earlier Espressif MCUs has not yet been verified)

---

## License

MIT License — see [`LICENSE`](https://github.com/Mr-PauI/ESP32-BinForge/blob/main/LICENSE) for details. 

This project includes esp_app_format.h from Espressif Systems, licensed under Apache 2.0.
