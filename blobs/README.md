# Blob Format

This directory contains pregenerated blobs of common sizes and minimal tooling related to **ESP32-BinForge injected blobs**.

Injected blobs are arbitrary data regions embedded into a merged ESP32 firmware image and later interpreted at runtime by firmware code.

See the [examples directory](https://github.com/Mr-PauI/ESP32-BinForge/tree/main/examples) for usage details.

---

## Blob Structure

Each injected blob follows a minimal structure:

- **Minimum size:** 32 bytes
- **Magic pattern:**
  - A single fixed byte pattern
  - Identical for all blobs
  - Occupies the entire 32-byte minimum size
- **CRC32:** Provided for the unmodified blob

There is no per-blob identifier, version field, checksum, or size field embedded in the blob.

---

## Magic Pattern

The magic pattern exists solely to allow runtime detection of injected data.

Important properties:

- The pattern does not vary between blobs
- The pattern is not unique per image
- Detection relies on scanning for this fixed pattern

Because the magic pattern size is 32 bytes, the smallest valid blob is 32 bytes.

---

## Usage

Blobs are intended to be:

- Injected using the BinForge CLI (or GUI)
- Evaluated at runtime using helper functions provided in `BinForge.h`
- Interpreted entirely by user code

BinForge does not impose any semantic meaning on blob contents.

---

## BinForge.h

`BinForge.h` provides lightweight helper definitions for detecting BinForge-injected blobs status at runtime.

It is designed to be:

- Header-only
- Platform-agnostic
- Usable from Arduino, PlatformIO, and bare ESP-IDF projects

---

## Responsibilities

`BinForge.h` is responsible for:

- Exposing minimal utilities without imposing policy

It does **not**:

- Parse blob payloads
- Enforce blob structure
- Allocate or manage memory

---

## Integration

Typical usage flow:

1. Include `BinForge.h` and a blob header at compile time
2. Inject data into the blob using the BinForge CLI or GUI
3. Flash firmware normally
4. Detect blob status at runtime using `BinForge.h` (see examples)
5. Interpret payload data in user code if the blob has been altered


