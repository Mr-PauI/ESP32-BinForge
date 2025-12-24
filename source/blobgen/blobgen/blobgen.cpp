#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstdint>
#include <cstring>
#include <string>

// Simple CRC32 implementation
uint32_t crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

// Default 32-byte magic pattern (replace with your own)
const uint8_t default_magic[32] = {
0x94,0x6C,0xAA,0xA0,0xDF,0x62,0xC4,0x02,
0x5F,0x8B,0x02,0xF4,0xA6,0x5E,0xEF,0x82,
0x37,0xE1,0xD5,0xAE,0x0A,0xA8,0x9C,0xF4,
0x7A,0xBB,0x90,0x49,0x70,0xB1,0x6A,0x3E };

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: blobgen <num_bytes> <output.h>\n";
        return 1;
    }

    size_t num_bytes = std::strtoul(argv[1], nullptr, 10);
    if (num_bytes < 32) {
        std::cerr << "Error: Minimum blob size is 32 bytes.\n";
        return 1;
    }

    std::string output_file = argv[2];
    std::vector<uint8_t> blob(num_bytes);

    // Copy magic pattern
    memcpy(blob.data(), default_magic, 32);

    // Fill remaining bytes with random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 255);
    for (size_t i = 32; i < num_bytes; ++i) {
        blob[i] = static_cast<uint8_t>(dis(gen));
    }

    // Compute CRC32
    uint32_t crc = crc32(blob.data(), blob.size());

    // Write .h file
    std::ofstream ofs(output_file);
    if (!ofs) {
        std::cerr << "Error: Cannot open output file.\n";
        return 1;
    }

    ofs << "#pragma once\n\n";
    ofs << "#include <cstdint>\n\n";
    ofs << "const uint8_t blob[" << num_bytes << "] = {";
    for (size_t i = 0; i < num_bytes; ++i) {
        if (i % 16 == 0) ofs << "\n    ";
        ofs << "0x" << std::hex << (int)blob[i];
        if (i + 1 != num_bytes) ofs << ", ";
    }
    ofs << "\n};\n\n";
    ofs << "const uint32_t blob_crc = 0x" << std::hex << crc << ";\n";

    std::cout << "Blob generated: " << output_file
        << " (" << num_bytes << " bytes, CRC=0x"
        << std::hex << crc << ")\n";

    return 0;
}
