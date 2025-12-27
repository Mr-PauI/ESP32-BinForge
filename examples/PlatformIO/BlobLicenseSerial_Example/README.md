



---



\## Intended Use Cases



This workflow is well suited for:



\- Device personalization

\- Serial number injection

\- Customer / licensee branding

\- API keys or credentials

\- Logos, assets, or ROM data

\- Final-stage firmware customization

\- Manufacturing or provisioning pipelines



All changes are applied \*\*after\*\*:

\- Compilation

\- Linking

\- ESP-IDF / Arduino / PlatformIO build

\- `esptool.py merge\_bin`



---



\## How It Works (High Level)



1\. The firmware embeds a structured data blob in flash.

2\. BinForge modifies the binary directly.

3\. BinForge recalculates the ESP image CRC.

4\. Optional SHA-256 image hashes are disabled or ignored for modified images.

5\. The resulting firmware image remains flashable and valid.



No custom bootloader or runtime patching is required.



---



\## Limitations



\- SHA-256 image hashes are currently disabled when data is modified.

\- Processed binaries should not be reprocessed repeatedly.

\- This demo focuses on \*\*data\*\*, not control flow or feature gating.



These limitations are by design for clarity and safety.

---



\## What This Demo Does \*\*Not\*\* Promise



This demo \*\*does not attempt to guarantee\*\* that all forms of binary data access will survive aggressive compiler optimization.



In particular:

\- Boolean feature flags

\- Compile-time-optimizable conditionals

\- Logic that the compiler can prove constant



may require additional techniques (such as `volatile` access patterns) to prevent optimization.



Those cases are intentionally \*\*out of scope\*\* for this initial demo.





---



\## License



This demo firmware is provided for demonstration purposes only.  

Refer to the main BinForge repository for licensing details.





---



\## Status



✅ Demonstration complete  

🧪 Tested against merged ESP32-S3 firmware images  

🚧 Additional features may be added in future revisions



