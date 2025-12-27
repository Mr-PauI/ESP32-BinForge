<img width="384" height="256" alt="image" src="https://github.com/user-attachments/assets/4a296cc2-7a35-4008-8cd0-3868c8d103b2" />

**ESP32-BinForge** is a small utility for modifying and revalidating ESP32 firmware .bin files. It supports injecting custom data into firmware images and regenerating CRCs, allowing binaries to remain flashable after modification.

The tool is designed for workflows where post-build customization is needed without recompiling firmware, such as embedding assets, configuration blobs, or replacing placeholder data.

The tool provides a reserved embedded region. The interpretation and use of that region is entirely user-defined.

Data regions are defined by a randomly generated [magic byte](https://github.com/Mr-PauI/ESP32-BinForge/blob/main/blobs/blob_32b.h) sequence. This sequence is intentionally non‑compressible and prevents toolchain or linker optimizations from collapsing or deduplicating the region during compilation. The minimum size is 32bytes but headers have been provided for many standard sizes and the included [blobgen](https://github.com/Mr-PauI/ESP32-BinForge/blob/main/bin/windows-x86_x64/blobgen.exe) tool can generate blobs of specific sizes.

Until user data is injected, the contents of this binary blob region should be considered undefined.  
  
---
Below is a screen shot from 3 concurrent terminal sessions for the same firmware. The left most image represents the included demo firmware
as compiled and flashed without modification. The next two terminal windows show the results after the compiled .bin file was processed with BinForge  
<img width="436" height="592" alt="BinForge_GUI" src="https://github.com/user-attachments/assets/b856f5f9-68b3-4a58-9e0f-50ca898ce36d" />



## Current Features

- **Data injection**
  - Injects user-supplied data into predefined regions of a firmware image
  - Automatically regenerates the CRC after injection

- **CRC-only mode**
  - Regenerates the CRC for binaries modified by external tools

- **SHA-256 handling**
  - Currently disables the SHA-256 check (required for modified images)
  - SHA-256 regeneration is planned for a future release
 
- **Examine .bin file contents**
  – Analyze ESP32 firmware images and display detailed segment information, including flash mode, chip type, entry address, segment load addresses, file offsets, sizes, CRC checks, and SHA-256 presence, providing a clear view of the binary contents.  - 
<img width="437" height="593" alt="BinForge_GUI" src="https://github.com/user-attachments/assets/fe370571-b20d-4fc5-bc33-bb11c703119f" />


---

## What It Does *Not* Do (Yet)

- Does **not** currently detect or report structural errors in firmware images
- Does **not** regenerate SHA-256 hashes
- Does **not** validate injected data correctness beyond checksum updates
- Does **not** strip SHA-256 hashes, meaning reprocessing a merged or validated binary should be currently avoided as the SHA-256 could be misinterpretted as a image header.

---

## Use Cases

- **Post-compilation firmware customization**  
  Modify firmware images after build without recompiling or relinking.

- **Asset or data blob injection into ESP32 binaries**  
  Embed ROM-resident assets such as logos, tables, lookup data, or game images.

- **Repairing CRCs after manual or automated binary edits**  
  Revalidate modified firmware images so they remain flashable.

- **Closed-source or restricted-source firmware workflows**  
  Customize or parameterize firmware images when source code is unavailable or rebuilding is impractical.

- **Manufacturing and deployment variants**  
  Generate multiple firmware variants (branding, configuration, region data) from a single base image.

- **Late-stage firmware changes**  
  Apply changes after code freeze, certification, or build system lock-down.

- **ROM-backed data for resource-constrained systems**  
  Store large static data in flash/ROM instead of consuming RAM.

- **Firmware experimentation and reverse-engineering workflows**  
  Safely modify binaries for testing, research, or exploratory development.

- **Emulator and retro firmware use cases**  
  Embed ROM images or game data directly into firmware for standalone emulation targets.

- **Licensing, provisioning, and device personalization**  
  Inject per-device keys, license numbers, API tokens, or other metadata without rebuilding the firmware. CRC checks allow firmware to detect whether injected data remains in its default state.

- **Tooling and automation pipelines** *(planned CLI tool)*  
  Integrate into scripts or CI systems for repeatable post-build firmware modification.



---

## Scope and Compatibility

- Based on ESP32 firmware image specifications  
- Intended for use with ESP32, ESP32-S2, ESP32-S3, ESP32-C2, ESP32-C3, ESP32-C5, ESP32-C6, ESP32-C61, ESP32-H2, ESP32-H21, ESP32-H4, ESP32-P4, and ESP32-S31
  (Compatibility with earlier Espressif MCUs has not yet been verified)

## Design Constraints

- BinForge operates on a single reserved data blob per firmware image.
- The blob is identified by a fixed 32-byte [magic pattern](https://github.com/Mr-PauI/ESP32-BinForge/blob/main/blobs/blob_32b.h). The minimum blob size is 32 bytes for this reason.
- Injection is performed in-place; firmware layout and segment sizes are not modified.
- The tool does not interpret blob contents — all semantics are defined by user code.
- Blob integrity is validated using a CRC32 generated at blob creation time.
- It is the user’s responsibility to ensure injected data fits within the reserved region.
- BinForge operates on unencrypted ESP32 firmware images.
- It is not a DRM or secure licensing solution.
- It does not operate on encrypted firmware. Any firmware encyrption must be applied *after* BinForge's modifcations have been made.

## License

MIT License — see [`LICENSE`](https://github.com/Mr-PauI/ESP32-BinForge/blob/main/LICENSE) for details. 

This project includes esp_app_format.h from Espressif Systems, licensed under Apache 2.0.
