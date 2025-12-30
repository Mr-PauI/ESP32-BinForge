
// ESP32-BinForge-CLI.cpp : Cross-platform CLI version
// MIT License, (c) 2025 MrPaul

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "esp_app_format.h"
#include "ESP32-BinForge-CLI.h"

using namespace std;

// ----- CONFIG: magic byte pattern -----
const uint8_t firmwareROM[32] = {
0x94,0x6C,0xAA,0xA0,0xDF,0x62,0xC4,0x02,
0x5F,0x8B,0x02,0xF4,0xA6,0x5E,0xEF,0x82,
0x37,0xE1,0xD5,0xAE,0x0A,0xA8,0x9C,0xF4,
0x7A,0xBB,0x90,0x49,0x70,0xB1,0x6A,0x3E };

// ----- PROTOTYPES -----
void merge_rom_cli(const std::string& inputBIN, const std::string& inputROM, const std::string& outputBIN);
void validate_esp32_bin(const std::string filename);
void analyze_bin(const std::string filename);



std::streamsize get_file_size(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    return file.tellg();
}

// --------------------
// MAIN
// --------------------
int main(int argc, char** argv)
{
    try {
        if (argc == 2) {
            // Only input → analyze binary file
            string input_bin = argv[1];
            analyze_bin(input_bin);
        }
        else if (argc == 3) {
            // Only input and output → regenerate CRC
            string input_bin = argv[1];
            string output_bin = argv[2];
            if (input_bin != output_bin) {
                // copy input to output first
                ifstream src(input_bin, ios::binary);
                ofstream dst(output_bin, ios::binary);
                dst << src.rdbuf();
            }
            validate_esp32_bin(output_bin);
            cout << "CRC regenerated for: " << output_bin << endl;
        }
        else if (argc == 4) {
            string input_bin = argv[1];
            string input_data = argv[2];
            string output_bin = argv[3];
            merge_rom_cli(input_bin, input_data, output_bin);
            cout << "Merge complete: " << output_bin << endl;
        }
        else {
            cerr << "Usage:\n"
                << "  ESP32-BinForge-CLI <input.bin>                          # Analyze .bin file\n"
                << "  ESP32-BinForge-CLI <input.bin> <output.bin>             # regenerate CRC only\n"
                << "  ESP32-BinForge-CLI <input.bin> <data.bin> <output.bin>  # merge data and regenerate CRC\n";
            return 0;
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

// --------------------
// Merge ROM function
// --------------------
void merge_rom_cli(const std::string& inputBIN, const std::string& inputROM, const std::string& outputBIN)
{
    if (inputBIN.empty() || inputROM.empty() || outputBIN.empty())
        throw runtime_error("Missing input/output files.");

    // Open files
    ifstream input_bin(inputBIN, ios::binary);
    ifstream input_rom(inputROM, ios::binary);

    if (!input_bin.is_open() || !input_rom.is_open())
        throw runtime_error("Failed to open one of the files.");
    ofstream output_bin(outputBIN, ios::binary);

    if (!output_bin.is_open())
        throw runtime_error("Failed to create output file.");

    cout << " Injecting data into bin file  \r\n";

    // Find the magic pattern
    int pattern_location = 0;
    char tempByte;
    int numBytesMatched = 0;
    while (input_bin.read(&tempByte, 1) && numBytesMatched < 32)
    {
        if ((uint8_t)tempByte == firmwareROM[numBytesMatched])
        {
            numBytesMatched++;
        }
        else
        {
            if ((uint8_t)tempByte != firmwareROM[0])
            {
                pattern_location++;
                pattern_location += numBytesMatched;
                numBytesMatched = 0;
            }
            else
            {
                numBytesMatched = 1;
            }
        }
    }

    if (numBytesMatched < 32)
        throw runtime_error("Reserved segment not found in input BIN file.");

    // Copy bytes up to the pattern
    input_bin.clear();
    input_bin.seekg(0, ios::beg);
    for (int i = 0; i < pattern_location; i++) {
        input_bin.read(&tempByte, 1);
        output_bin.write(&tempByte, 1);
    }

    // Inject new data
    while (input_rom.read(&tempByte, 1)) {
        output_bin.write(&tempByte, 1);
    }

    // Copy remaining bytes of input BIN
    input_bin.seekg(pattern_location + 32, ios::beg);
    while (input_bin.read(&tempByte, 1)) {
        output_bin.write(&tempByte, 1);
    }

    input_bin.close();
    input_rom.close();
    output_bin.close();

    // Regenerate CRC
    validate_esp32_bin(outputBIN);
}


// --------------------
// Regenerate CRC
// --------------------
void validate_esp32_bin(const std::string filename)
{
    cout << "-----------------------------\r\n";
    cout << "  Revalidating esp32 binary  \r\n";
    // Regenerate CRC and disable the Sha256 checksum
    // Step1: Read the entire outBin into memory
    uint8_t stats_images = 0;
    uint8_t stats_errors = 0;
    uint8_t stats_sha256_disabledCount = 0;
    uint8_t stats_crc_regenCount = 0;
    uint8_t* bin_data;
    uint8_t crc_value;
    // Get the file size, there is a faster way to do this, but for now this works
    uint32_t filesize = (uint32_t)get_file_size(filename);
    ifstream input_bin(filename, ios::in | ios::binary);
    bin_data = (uint8_t*)malloc(filesize);
    input_bin.read((char*)bin_data, filesize);
    input_bin.close();
    // Step2: Compute the CRC value's for each segment for each image in the file
    uint32_t image_header = 0;
    uint32_t file_position = 0; // position us at the beginning of the first segment
    for (;file_position < filesize;)
    {
        if (bin_data[file_position] == 0xe9)
        { // we have located an image 
            cout << "-----------------------------\r\n";
            cout << "Image found at offset 0x" << std::hex << file_position << std::dec <<  "\r\n";
            esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(bin_data + file_position);
            cout << "Entry Address: ";
            cout << std::hex << hdr->entry_addr << std::dec;
            cout << "\r\n";

            stats_images++;
            image_header = file_position;
            file_position += 24; // position us at the beginning of the first segment
            uint8_t segment_count = bin_data[image_header + 1]; // 2nd byte contains number of segments
            // Log number of segments
            cout << "     Segments: " << std::to_string(segment_count) << "\r\n";
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
                //cout << " Segment #" << std::to_string(j) << "   File Offset: ";
                //cout << std::hex << file_position << std::dec;
                //cout << "    Size: " << std::to_string(segment_size) << " bytes\r\n";
                file_position += 8; // skip over segment header
                for (unsigned int i = 0; i < segment_size; i++)
                {
                    crc_value ^= bin_data[file_position];
                    file_position++;
                }
            }
            uint32_t padding = (16 - ((file_position + 1) % 16)) % 16; // imagesize+1 for crc, happens to be our value in file_position. Since all images are 16-byte aligned/padded we do not need to consider relative image positions
            uint32_t crc_location = (file_position)+padding; // length of image prior to this+image_header position == file_position
            uint8_t old_crc = bin_data[crc_location];
            cout << " Image original CRC:" << std::to_string(old_crc) << "\r\n";
            cout << " Image computed CRC:" << std::to_string(crc_value) << "\r\n";
            // Step3: Update the CRC value, disable sha256 ; forced for now - to avoid sha256 errors on duplicate crcs
            //if (crc_value != bin_data[crc_location])
            {
                if (crc_location > filesize)
                {
                    stats_errors++;
                    crc_location = filesize - 33;// just a temporary thing until the crc locating is correct
                    cout << " ERROR!! Expectd CRC not located within file\r\n";
                }
                else
                {
                    stats_crc_regenCount++;
                    bin_data[crc_location] = crc_value;
                }
                // Step4: Change checksum to simple CRC mode
                if (bin_data[image_header + 23])
                {
                    cout << "   Image has has sha256\r\n";
                    bin_data[image_header + 23] = 0; // disables sha256
                    file_position = crc_location + 32 + 1;
                    cout << "   Image sha256 has been disabled\r\n";
                    stats_sha256_disabledCount++;
                }
                else
                {
                    cout << "   Image had no sha256 to disable\r\n";
                    file_position = crc_location + 1;
                }
            }
        }
        else
            file_position += 16;// image headers are always 16-byte aligned
    }
    // Step5: Write output binary
    ofstream output_bin(filename, ios::out | ios::binary);
    cout << "-----------------------------\r\n";
    cout << "  Revalidation completed: " + std::to_string(filesize) + " bytes written\r\n";
    output_bin.write((char*)bin_data, filesize);
    output_bin.close();
    free(bin_data);
}
 
void analyze_bin(const std::string filename)
{
    cout <<  "-----------------------------\r\n";
    cout <<  "  Analyze esp32 binary  \r\n";

    // Regenerate CRC and disable the Sha256 checksum
    // Step1: Read the entire outBin into memory
    uint8_t stats_images = 0;
    uint8_t stats_errors = 0;
    uint8_t stats_sha256_disabledCount = 0;
    uint8_t stats_crc_regenCount = 0;
    uint8_t* bin_data;
    uint8_t crc_value;
    // Get the file size, there is a faster way to do this, but for now this works
    uint32_t filesize = (uint32_t)get_file_size(filename);
    ifstream input_bin(filename, ios::in | ios::binary);
    bin_data = (uint8_t*)malloc(filesize);
    input_bin.read((char*)bin_data, filesize);
    input_bin.close();
    // Step2: Compute the CRC value's for each segment for each image in the file
    uint32_t image_header = 0;
    uint32_t file_position = 0; // position us at the beginning of the first segment
    for (;file_position < filesize;)
    {
        if (bin_data[file_position] == 0xe9)
        { // we have located an image
            cout <<  "-----------------------------\r\n";
            cout << "Image found at offset 0x" << std::hex << file_position << std::dec << "\r\n";
            esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(bin_data + file_position);
            // Log SPI mode
            cout <<  "   Flash Mode: ";
            switch (hdr->spi_mode)
            {
            case 0:
                cout <<  "QIO\r\n";
                break;
            case 1:
                cout <<  "QOUT\r\n";
                break;
            case 2:
                cout <<  "DIO\r\n";
                break;
            case 3:
                cout <<  "DOUT\r\n";
                break;
            case 4:
                cout <<  "FAST READ\r\n";
                break;
            case 5:
                cout <<  "SLOW READ\r\n";
                break;
            default:
                cout <<  "Unknown\r\n";
                break;
            }
            cout <<  "      Chip ID: ";
            switch (hdr->chip_id)
            {
            case ESP_CHIP_ID_ESP32:
                cout <<  "ESP32\r\n";
                break;
            case ESP_CHIP_ID_ESP32S2:
                cout <<  "ESP32-S2\r\n";
                break;
            case ESP_CHIP_ID_ESP32C3:
                cout <<  "ESP32-C3\r\n";
                break;
            case ESP_CHIP_ID_ESP32S3:
                cout <<  "ESP32-S3\r\n";
                break;
            case ESP_CHIP_ID_ESP32C2:
                cout <<  "ESP32-C2\r\n";
                break;
            case ESP_CHIP_ID_ESP32C6:
                cout <<  "ESP32-C6\r\n";
                break;
            case ESP_CHIP_ID_ESP32H2:
                cout <<  "ESP32-H2\r\n";
                break;
            case ESP_CHIP_ID_ESP32P4:
                cout <<  "ESP32-P4\r\n";
                break;
            case ESP_CHIP_ID_ESP32C5:
                cout <<  "ESP32-C5\r\n";
                break;
            case ESP_CHIP_ID_ESP32C61:
                cout <<  "ESP32-C61\r\n";
                break;
            case ESP_CHIP_ID_ESP32H21:
                cout <<  "ESP32-H21\r\n";
                break;
            case ESP_CHIP_ID_ESP32H4:
                cout <<  "ESP32-H4\r\n";
                break;
            case ESP_CHIP_ID_ESP32S31:
                cout <<  "ESP32-S31\r\n";
                break;
            default:
                cout <<  "Unknown\r\n";
                break;
            }
            cout <<  "Entry Address: ";
            cout << std::hex << hdr->entry_addr << std::dec;
            cout <<  "\r\n";

            stats_images++;
            image_header = file_position;
            file_position += 24; // position us at the beginning of the first segment
            uint8_t segment_count = bin_data[image_header + 1]; // 2nd byte contains number of segments
            // Log number of segments
            cout <<  "     Segments: " + std::to_string(segment_count) + "\r\n";
            cout <<  "-----------------------\r\n";
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
                cout <<  " Segment #" + std::to_string(j) + "\r\n     Load Addr: 0x";
                cout << std::hex << segment_addr << std::dec;
                cout <<  "\r\n   File Offset: 0x";
                cout << std::hex << file_position << std::dec;
                cout <<  "\r\n          Size: " + std::to_string(segment_size) + " bytes\r\n";
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
            cout <<  " Image original CRC:" + std::to_string(old_crc) + "\r\n";
            cout <<  " Image computed CRC:" + std::to_string(crc_value) + "\r\n";
            // Step3: Update the CRC value, forced for now - to avoid sha256 errors on duplicate crcs
            if (crc_value != bin_data[crc_location])
                cout <<  " **** WARNING **** CRC MISMATCH\r\n";


            if (crc_location > filesize)
            {
                stats_errors++;
                crc_location = filesize - 33;// just a temporary thing until the crc locating is correct
                cout <<  " ERROR!! Expectd CRC not located within file. Segment header faulty or file incomplete.\r\n";
            }
            else
            {
                stats_crc_regenCount++;
                bin_data[crc_location] = crc_value;
            }
            // Step4: Change checksum to simple CRC mode
            if (bin_data[image_header + 23])
            {
                cout <<  "   Image has has sha256\r\n";
                file_position = crc_location + 32 + 1;
            }
            else
            {
                cout <<  "   Image has no sha256\r\n";
                file_position = crc_location + 1;
            }

        }
        else
            file_position += 16;// image headers are always 16-byte aligned
    }
    // End of file analysis
    cout <<  "-----------------------------\r\n";
    cout <<  "  Total binary file size: " + std::to_string(filesize) + " bytes \r\n";
    free(bin_data);
} 
