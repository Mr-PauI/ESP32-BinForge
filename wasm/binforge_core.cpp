#include "binforge_core.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include "binforge_core.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif




// ------------------------------------------------------
// JS-side logging hook (we will implement in JS later)
// ------------------------------------------------------
#ifdef __EMSCRIPTEN__
EM_JS(void, js_log, (const char* msg), {
    if (typeof window !== "undefined" && window.binforgeLog) {
        window.binforgeLog(UTF8ToString(msg));
    }
 else {
  console.log(UTF8ToString(msg));
}
    });
#endif

// ------------------------------------------------------
// printf-style logger
// ------------------------------------------------------
void bf_log(const char* fmt, ...)
{
    char buffer[512];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

#ifdef __EMSCRIPTEN__
    js_log(buffer);
#else
    printf("%s", buffer);
#endif
}

// ------------------------------------------------------
// Dummy analysis function
// ------------------------------------------------------
/*
void analyze_esp32_binary(const uint8_t* bin_data, uint32_t filesize)
{
    

    if (!bin_data || filesize == 0)
    {
        bf_log("ERROR: Invalid input buffer\n");
        return;
    }

    bf_log("First byte: 0x%02X\n", bin_data[0]);
    bf_log("Analysis complete (dummy stage)\n");
}*/

void analyze_esp32_binary(const uint8_t* bin_data, uint32_t filesize)
{
    bf_log("-----------------------------\n");
    bf_log("  Analyze esp32 binary  \n");
    uint8_t stats_images = 0;
    uint8_t stats_errors = 0;
    uint8_t stats_sha256_disabledCount = 0;
    uint8_t stats_crc_regenCount = 0;
    uint8_t crc_value;
    // Step 1: Process header and compute the CRC value's for each segment for each image in the file
    uint32_t image_header = 0;
    uint32_t file_position = 0; // position us at the beginning of the first segment
    for (;file_position < filesize;)
    {
        if (bin_data[file_position] == 0xe9)
        { // we have located an image
            bf_log("-----------------------------\n");
            bf_log("Image found at offset 0x%x\n",file_position);
            //esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(bin_data + file_position);
            esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(const_cast<uint8_t*>(bin_data + file_position));
            // Log SPI mode

            bf_log("   Flash Mode: ");
            switch (hdr->spi_mode)
            {
            case 0:
                bf_log("QIO\n");
                break;
            case 1:
                bf_log("QOUT\n");
                break;
            case 2:
                bf_log("DIO\n");
                break;
            case 3:
                bf_log("DOUT\n");
                break;
            case 4:
                bf_log("FAST READ\n");
                break;
            case 5:
                bf_log("SLOW READ\n");
                break;
            default:
                bf_log("Unknown\n");
                break;
            }
            bf_log("      Chip ID: ");
            switch (hdr->chip_id)
            {
            case ESP_CHIP_ID_ESP32:
                bf_log("ESP32\n");
                break;
            case ESP_CHIP_ID_ESP32S2:
                bf_log("ESP32-S2\n");
                break;
            case ESP_CHIP_ID_ESP32C3:
                bf_log("ESP32-C3\n");
                break;
            case ESP_CHIP_ID_ESP32S3:
                bf_log("ESP32-S3\n");
                break;
            case ESP_CHIP_ID_ESP32C2:
                bf_log("ESP32-C2\n");
                break;
            case ESP_CHIP_ID_ESP32C6:
                bf_log("ESP32-C6\n");
                break;
            case ESP_CHIP_ID_ESP32H2:
                bf_log("ESP32-H2\n");
                break;
            case ESP_CHIP_ID_ESP32P4:
                bf_log("ESP32-P4\n");
                break;
            case ESP_CHIP_ID_ESP32C5:
                bf_log("ESP32-C5\n");
                break;
            case ESP_CHIP_ID_ESP32C61:
                bf_log("ESP32-C61\n");
                break;
            case ESP_CHIP_ID_ESP32H21:
                bf_log("ESP32-H21\n");
                break;
            case ESP_CHIP_ID_ESP32H4:
                bf_log("ESP32-H4\n");
                break;
            case ESP_CHIP_ID_ESP32S31:
                bf_log("ESP32-S31\n");
                break;
            default:
                bf_log("Unknown\n");
                break;
            }
            bf_log("Entry Address: 0x%080\nx", hdr->entry_addr);

            stats_images++;
            image_header = file_position;
            file_position += 24; // position us at the beginning of the first segment
            uint8_t segment_count = bin_data[image_header + 1]; // 2nd byte contains number of segments
            // Log number of segments
            bf_log("     Segments: %d\n", segment_count);
            bf_log("-----------------------\n");
            uint32_t segment_size = 0;
            uint32_t segment_addr = 0;
            crc_value = 0xef;
            for (int j = 0; j < segment_count; j++)
            {
                segment_addr =
                    (uint32_t)bin_data[file_position] |
                    ((uint32_t)bin_data[file_position + 1] << 8) |
                    ((uint32_t)bin_data[file_position + 2] << 16) |
                    ((uint32_t)bin_data[file_position + 3] << 24);
                segment_size =
                    (uint32_t)bin_data[file_position + 4] |
                    ((uint32_t)bin_data[file_position + 5] << 8) |
                    ((uint32_t)bin_data[file_position + 6] << 16) |
                    ((uint32_t)bin_data[file_position + 7] << 24);
                bf_log(" Segment #%d\n     Load Addr: 0x%x\n",j, segment_addr);
                bf_log("   File Offset: 0x%08x\n", file_position);
                bf_log("          Size: %d bytes\n",segment_size);
                file_position += 8; // skip over segment header
                for (unsigned int i = 0; i < segment_size; i++)
                {
                    crc_value ^= bin_data[file_position];
                    file_position++;
                }
            }
            uint32_t padding = (16 - ((file_position + 1) % 16)) % 16; // imagesize+1 for crc, happens to be our value in file_position. Since all images are 16-byte aligned/padded we do not need to consider relative image positions
            uint32_t crc_location = (file_position)+padding;
            uint8_t old_crc = bin_data[crc_location];
            bf_log(" Image original CRC: 0x%02x\n",old_crc);
            bf_log(" Image computed CRC: 0x%02x\n", crc_value);
            // Step2: Compute and if desired update the CRC value, forced for now - to avoid sha256 errors on duplicate crcs
            if (crc_value != bin_data[crc_location])
                bf_log(" **** WARNING **** CRC MISMATCH\n");


            if (crc_location > filesize)
            {
                stats_errors++;
                crc_location = filesize - 33;// just a temporary thing until the crc locating is correct
                bf_log(" ERROR!! Expectd CRC not located within file. Segment header faulty or file incomplete.\n");
            }
            else
            {
                /*DISABLED, this would normally place the corrected crc in the bin_data before saving
                stats_crc_regenCount++;
                bin_data[crc_location] = crc_value;*/
            }
            // Step3: Detect sha256 (and if desired disable/change checksum to simple CRC mode)
            if (bin_data[image_header + 23])
            {
                bf_log("   Image has sha256\n");
                file_position = crc_location + 32 + 1;
            }
            else
            {
                bf_log("   Image has no sha256\n");
                file_position = crc_location + 1;
            }

        }
        else
            file_position += 16;// image headers are always 16-byte aligned
    }
    // End of file analysis
    bf_log("-----------------------------\n");
    bf_log("  Total binary file size: %d bytes \n",filesize);
}