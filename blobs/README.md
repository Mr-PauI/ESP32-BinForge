\# Blob Format



This directory contains pregenerated blobs of common sizes and some minimal tools related to \*\*ESP32-BinForge injected blobs\*\*.



Injected blobs are arbitrary data regions embedded into a merged ESP32 firmware image and later interpretted at runtime by firmware code.



See examples for usage details.



---



\## Blob Structure



Each injected blob follows a minimal structure:



\- \*\*Minimum size:\*\* 32 bytes

\- \*\*Magic pattern:\*\*  

&nbsp; - A single fixed byte pattern  

&nbsp; - Identical for all blobs  

&nbsp; - Occupies the entire 32-byte minimum size

\- \*\*CRC32 for unmodified blob\*\*



There is \*\*no per-blob identifier\*\*, version field, checksum, or size field embedded in the blob.



---



\## Magic Pattern



The magic pattern exists solely to allow runtime detection of injected data.



Important properties:



\- The pattern does \*\*not\*\* vary between blobs

\- The pattern is \*\*not unique per image\*\*

\- Detection relies on scanning for this fixed pattern



Because the minimum blob size is 32 bytes, the magic occupies the entire smallest valid blob.



---



\## Usage



Blobs are intended to be:



\- Injected using the BinForge CLI

\- Evaluated at runtime using helper functions provided in `BinForge.h`

\- Interpreted entirely by user code



BinForge does not impose any semantic meaning on blob contents.



\## BinForge.h



`BinForge.h` provides lightweight helper definitions for locating and accessing BinForge-injected blobs at runtime.



It is designed to be:



\- Header-only

\- Platform-agnostic

\- Usable from Arduino, PlatformIO and bare ESP-IDF projects



---



\## Responsibilities



`BinForge.h` is responsible for:



\- Defining the blob magic pattern

\- Exposing minimal utilities without imposing policy



It does \*\*not\*\*:



\- Parse blob payloads

\- Enforce blob structure

\- Allocate or manage memory



\## Integration



Typical usage flow:



1\. Inject blob into firmware using BinForge CLI tool(or GUI)

2\. Flash firmware normally

3\. Detect blob status at runtime using `BinForge.h` (see examples)

4\. Interpret payload data in user code

