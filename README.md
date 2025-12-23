# ESP32-BinForge

**ESP32-BinForge** is a small utility for modifying and revalidating ESP32 firmware `.bin` files. It supports injecting custom data into firmware images while automatically updating the CRC to keep binaries flashable after modification.

The tool is designed for workflows where post-build customization is needed without recompiling firmware, such as embedding assets, configuration blobs, or replacing placeholder data.

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
