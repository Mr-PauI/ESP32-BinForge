// BlobData.h
#pragma once
#include <cstdint>
#include <cstddef>
// Modify the line below to point to your actual blob data header
#include "blob_32b.h"

// Compute CRC32 at runtime, ensuring the compiler can't optimize it out
inline uint32_t ComputeBlobCRC(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

// Cross-platform force-volatile read to prevent dead-code elimination
inline volatile bool VerifyBlob() {
    // read through volatile pointer ensures compiler can't optimize away the blob
    volatile const uint8_t* ptr = blob;
    return ComputeBlobCRC(const_cast<const uint8_t*>(ptr), sizeof(blob)) == blob_crc;
}