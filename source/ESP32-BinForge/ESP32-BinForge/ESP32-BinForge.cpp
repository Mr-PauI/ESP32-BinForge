// ESP32-BinForge (c)2025 MrPaul
/*
 *    MIT License
 *
 *    Copyright(c) 2025 Mr - PauI(https://github.com/Mr-PauI/)
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy
 *    of this software and associated documentation files(the "Software"), to deal
 *    in the Software without restriction, including without limitation the rights
 *    to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 *    copies of the Software, and to permit persons to whom the Software is
 *    furnished to do so, subject to the following conditions :
 *
 *    The above copyright notice and this permission notice shall be included in all
 *    copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *    SOFTWARE.
*/
#include "framework.h"
#include "ESP32-BinForge.h"
#include "esp_app_format.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void MergeROM(HWND);
uint8_t compute_esp32s3_crc(uint8_t* data, int count);
uint8_t xor_sum_of_sections(const std::string& file_path);
void validate_esp32_bin(const std::string filename);
void revalidate_crc_nosha256(HWND hWnd);
using namespace std;

// Function to show the open file dialog and return the selected file name
std::string ShowOpenFileDialogBIN(HWND hwnd) {
    OPENFILENAME openFileDialog;       // Common dialog box structure
    TCHAR szFile[260] = { 0 };         // Buffer for file name

    ZeroMemory(&openFileDialog, sizeof(openFileDialog));
    // Initialize OPENFILENAME structure
    openFileDialog.lStructSize = sizeof(openFileDialog); // Size of the structure
    openFileDialog.hwndOwner = hwnd;                     // Owner window handle
    openFileDialog.lpstrFile = szFile;                   // File name buffer
    openFileDialog.lpstrFile[0] = '\0';                  // Initialize the first character to zero
    openFileDialog.nMaxFile = sizeof(szFile) / sizeof(szFile[0]); // Size of the file name buffer
    openFileDialog.lpstrFilter = _T("BIN Files\0*.bin\0All Files\0*.*\0"); // File type filters
    openFileDialog.nFilterIndex = 1;                     // Index of the default filter
    openFileDialog.lpstrFileTitle = NULL;                // File title buffer (optional)
    openFileDialog.nMaxFileTitle = 0;                    // Size of the file title buffer
    openFileDialog.lpstrInitialDir = NULL;               // Initial directory (optional)
    openFileDialog.lpstrTitle = _T("Select input .BIN file");
    openFileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // Flags for file dialog behavior

    // Display the Open dialog box
    if (GetOpenFileName(&openFileDialog) == TRUE) {
        // Convert TCHAR to std::string
#ifdef UNICODE
        std::wstring ws(openFileDialog.lpstrFile);
        return std::string(ws.begin(), ws.end());
#else
        return std::string(openFileDialog.lpstrFile);
#endif
    }

    // If the user cancels the dialog, return an empty string
    return "";
}

// Function to show the open file dialog and return the selected file name
std::string ShowOpenFileDialog(HWND hwnd) {
    OPENFILENAME openFileDialog;       // Common dialog box structure
    TCHAR szFile[260] = { 0 };         // Buffer for file name

    ZeroMemory(&openFileDialog, sizeof(openFileDialog));
    // Initialize OPENFILENAME structure
    openFileDialog.lStructSize = sizeof(openFileDialog); // Size of the structure
    openFileDialog.hwndOwner = hwnd;                     // Owner window handle
    openFileDialog.lpstrFile = szFile;                   // File name buffer
    openFileDialog.lpstrFile[0] = '\0';                  // Initialize the first character to zero
    openFileDialog.nMaxFile = sizeof(szFile) / sizeof(szFile[0]); // Size of the file name buffer
    openFileDialog.lpstrFilter = _T("All Files\0*.*\0Image Files\0*.bmp;*.png;*.jpg;*.jpeg\0Sound Files\0*.wav;*.mp3\0Data Files\0*.dat;*.bin;*.obb;*.blob\0"); // File type filters
    openFileDialog.nFilterIndex = 1;                     // Index of the default filter
    openFileDialog.lpstrFileTitle = NULL;                // File title buffer (optional)
    openFileDialog.nMaxFileTitle = 0;                    // Size of the file title buffer
    openFileDialog.lpstrInitialDir = NULL;               // Initial directory (optional)
    openFileDialog.lpstrTitle = _T("Select data to inject");
    openFileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; // Flags for file dialog behavior

    // Display the Open dialog box
    if (GetOpenFileName(&openFileDialog) == TRUE) {
        // Convert TCHAR to std::string
#ifdef UNICODE
        std::wstring ws(openFileDialog.lpstrFile);
        return std::string(ws.begin(), ws.end());
#else
        return std::string(openFileDialog.lpstrFile);
#endif
    }

    // If the user cancels the dialog, return an empty string
    return "";
}

// Function to show the save file dialog and return the selected file 
std::string ShowSaveFileDialog(HWND hwnd) {
    OPENFILENAME saveFileDialog;       // Common dialog box structure
    TCHAR szFile[260] = _T("output.bin"); // Buffer for file name with a suggested filename

    ZeroMemory(&saveFileDialog, sizeof(saveFileDialog));
    // Initialize OPENFILENAME structure
    saveFileDialog.lStructSize = sizeof(saveFileDialog); // Size of the structure
    saveFileDialog.hwndOwner = hwnd;                     // Owner window handle
    saveFileDialog.lpstrFile = szFile;                   // File name buffer
    saveFileDialog.nMaxFile = sizeof(szFile) / sizeof(szFile[0]); // Size of the file name buffer
    saveFileDialog.lpstrFilter = _T("Binary Files\0*.bin\0All Files\0*.*\0"); // File type filters
    saveFileDialog.nFilterIndex = 1;                     // Index of the default filter
    saveFileDialog.lpstrFileTitle = NULL;                // File title buffer (optional)
    saveFileDialog.nMaxFileTitle = 0;                    // Size of the file title buffer
    saveFileDialog.lpstrInitialDir = NULL;               // Initial directory (optional)
    saveFileDialog.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT; // Flags for file dialog behavior

    // Display the Save dialog box
    if (GetSaveFileName(&saveFileDialog) == TRUE) {
        // Convert TCHAR to std::string
#ifdef UNICODE
        std::wstring ws(saveFileDialog.lpstrFile);
        return std::string(ws.begin(), ws.end());
#else
        return std::string(saveFileDialog.lpstrFile);
#endif
    }

    // If the user cancels the dialog, return an empty string
    return "";
}

// Regarding signature size
// BinForge uses a fixed 32 - byte random signature to locate the injection region.
// This provides 256 bits of entropy, making accidental matches in firmware binaries effectively impossible.
// Earlier versions successfully used smaller signatures; 32 bytes was chosen as a conservative, future - proof default.
const uint8_t magic_pattern[32] = {
0x94,0x6C,0xAA,0xA0,0xDF,0x62,0xC4,0x02,
0x5F,0x8B,0x02,0xF4,0xA6,0x5E,0xEF,0x82,
0x37,0xE1,0xD5,0xAE,0x0A,0xA8,0x9C,0xF4,
0x7A,0xBB,0x90,0x49,0x70,0xB1,0x6A,0x3E };
// BMO byte pattern
 

// Message handler for Merge ROM
void MergeROM(HWND hWnd)
{
    string inputBIN=ShowOpenFileDialogBIN(hWnd);
    string inputROM = ShowOpenFileDialog(hWnd);
    string outputBIN = ShowSaveFileDialog(hWnd);    
    int filesize = 0;

    if (inputBIN.empty() || inputROM.empty() || outputBIN.empty())
    {
        MessageBoxW(
            hWnd,
            L"Operation cancelled. Pne or more files were not selected.\n\n"
            L"Please select:\n"
            L"* Input firmware (.bin)\n"
            L"* Data file to inject (.*)\n"
            L"* Output firmware (.bin)",
            L"",
            MB_OK | MB_ICONWARNING
        );
        return;
    }

    //check if any of the required filenames was not provided
    if (inputROM.empty()||(inputBIN.empty())||(outputBIN.empty()))
        return;

    ofstream output_bin(outputBIN, ios::out | ios::binary);
    ifstream input_bin(inputBIN, ios::in | ios::binary);
    int pattern_location = 0;
    char tempByte;
    int numBytesMatched = 0; // when this reaches the byte pattern count we have found the segment to begin replacing
    while ((!input_bin.eof()) && (numBytesMatched < 32))    
    {
        input_bin.read(&tempByte, 1);
        if ((uint8_t)tempByte == magic_pattern[numBytesMatched])
        {
            numBytesMatched++;
        }
        else
        {   // if this isn't the start of a pattern
            if (tempByte != magic_pattern[0])
            {
                pattern_location++;
                pattern_location += numBytesMatched;// add any bytes we had matched but not counted
                numBytesMatched = 0; // nothing is matching what we expectd
            }
            else
            {   // its the start of our pattern, so lets count it
                numBytesMatched=1;
            }

        }
    }
    input_bin.close();
    // Ensure we found the pattern
    if (numBytesMatched < 32)
    {
        MessageBoxW(
            hWnd,
            L"Operation could not be completed\n\n"
            L"Reserved segment could not be found\n"
            L"Ensure you are using the correct input file\n",
            L"",
            MB_OK | MB_ICONWARNING
        );
        return;
    }
    // Perform the merge operation
    // Copy bytes up to the pattern position to the output bin
    input_bin.open(inputBIN, ios::in | ios::binary);
    for (int i = 0; i < pattern_location; i++)
    {
        input_bin.read(&tempByte, 1); // copy byte from input bin
        output_bin.write(&tempByte, 1); // into output bin
        filesize++;
    }
    ifstream input_rom(inputROM, ios::in | ios::binary);
    while (!input_rom.eof())
    {
        input_bin.read(&tempByte, 1); // move forward one byte in input bin
        input_rom.read(&tempByte, 1); // copy byte from input rom
        output_bin.write(&tempByte, 1);// into the output bin
        filesize++;
    }
    // Copy the remaining bytes in the input bim
    while (!input_bin.eof())
    {
        input_bin.read(&tempByte, 1); /// copy byte from input bin
        if (!input_bin.eof())
        {
            output_bin.write(&tempByte, 1);// into the output bin
            filesize++;
        }
    }
    // Cleanup after
    input_rom.close();
    output_bin.close();
    input_bin.close();

    // Regenerate CRC and disable the Sha256 checksum
    validate_esp32_bin(outputBIN);
    MessageBox(hWnd, L"Merge ROM Complete!", L"Success!", MB_ICONEXCLAMATION | MB_OK);
    return;
    // Step1: Read the entire outBin into memory
    //input_bin.open(inputBIN, ios::in | ios::binary);
    input_bin.open(outputBIN, ios::in | ios::binary);
    uint8_t* bin_data;
    bin_data = (uint8_t*)malloc(filesize);
    input_bin.read((char*)bin_data, filesize);    
    input_bin.close();
    // Step2: Compute the CRC value's for each segment
    uint8_t file_position = 24; // position us at the beginning of the first segment
    uint8_t segment_count = bin_data[1]; // 2nd byte contains number of segments
    uint32_t segment_size = 0;
    uint8_t crc_value=0xef;
    for (int j = 0; j < segment_count; j++)
    {
        segment_size = (uint32_t)bin_data[file_position+4];
        file_position += 8; // skip over segment header
        for (int i = 0; i < segment_size; i++)
        {
            crc_value ^= bin_data[file_position];
            file_position++;
        }
    }
    crc_value= xor_sum_of_sections(outputBIN);
   // crc_value ^= 0xef;
    //crc_value ^= 0x0d;
    //for (int i = 24 + 8; i < filesize - 33; i++)
    //    crc_value ^= bin_data[i];
    // Step3: Update the CRC value
    bin_data[filesize - 33] = crc_value;
    // Step4: Change checksum to simple CRC mode
    bin_data[23] = 0;
    // Step5: Write output binary
    output_bin.open(outputBIN, ios::out | ios::binary);
    output_bin.write((char*)bin_data, filesize);
    output_bin.close();
    free(bin_data);
}

uint8_t compute_esp32s3_crc(uint8_t *data,int count)
{
    uint8_t crc_value = 0xEF; // value found in https://docs.espressif.com/projects/esptool/en/latest/esp32s3/advanced-topics/firmware-image-format.html
    //uint8_t crc_value = 0;
    for (int i = 0; i < count; i++)
        crc_value ^= data[i];
    return crc_value;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ESP32BINFORGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ESP32BINFORGE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ESP32BINFORGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDI_ESP32BINFORGE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));

   HWND hWnd = CreateWindowW(
       szWindowClass,
       szTitle,
       WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0,
       // CW_USEDEFAULT, 0, // default window size
       500, 200,
       nullptr,
       hMenu,        
       hInstance,
       nullptr
   );

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_FILE_REVALIDATE:
                revalidate_crc_nosha256(hWnd);
                break;
            case IDM_MERGEROM:
                MergeROM(hWnd);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


uint8_t xor_sum_of_sections(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << file_path << std::endl;
        return 0;
    }

    // Read number of sections from the second byte
    file.seekg(1, std::ios::beg);
    uint8_t num_sections;
    file.read(reinterpret_cast<char*>(&num_sections), sizeof(num_sections));
    std::cout << "Number of sections: " << static_cast<int>(num_sections) << std::endl;

    // Initial offset to start reading sections
    std::streamoff offset = 24;
    uint32_t xor_sum = 0xEF;

    for (uint8_t i = 0; i < num_sections; ++i) {
        file.seekg(offset, std::ios::beg);
        uint32_t header[2];
        file.read(reinterpret_cast<char*>(header), sizeof(header));
        uint32_t load_addr = header[0];
        uint32_t data_length = header[1];

       // std::cout << "Section " << static_cast<int>(i + 1) << ": addr = " << std::hex << load_addr << ", Length = " << data_length << std::endl;

        std::vector<uint8_t> data(data_length);
        file.read(reinterpret_cast<char*>(data.data()), data_length);

        for (uint8_t byte : data) {
            xor_sum ^= byte;
        }

        offset += 8 + data_length;
    }
    return xor_sum;
}

void revalidate_crc_nosha256(HWND hWnd)
{
    string inputBIN = "";
    inputBIN = ShowOpenFileDialogBIN(hWnd);
    if (inputBIN != "")
    {
        validate_esp32_bin(inputBIN);
        MessageBox(hWnd, L"File CRC regenerated, SHA256 hash disabled", L"Done!", MB_ICONEXCLAMATION | MB_OK);
    }
}

void validate_esp32_bin(const std::string filename)
{    
    ifstream input_bin(filename, ios::in | ios::binary);
    // Regenerate CRC and disable the Sha256 checksum
    // Step1: Read the entire outBin into memory
    uint8_t* bin_data;
    uint8_t crc_value;
    // Get the file size, there is a faster way to do this, but for now this works
    uint32_t filesize=0;
    char tempByte;
    while (!input_bin.eof())
    {
        input_bin.read(&tempByte, 1); /// copy byte from input bin
        if (!input_bin.eof())
        {
            filesize++;
        }
    }
    input_bin.close();
    input_bin.open(filename, ios::in | ios::binary);
    bin_data = (uint8_t*)malloc(filesize);
    input_bin.read((char*)bin_data, filesize);
    input_bin.close();
    // Step2: Compute the CRC value's for each segment
    crc_value = xor_sum_of_sections(filename);
     // Step3: Update the CRC value
    bin_data[filesize - 33] = crc_value;
    // Step4: Change checksum to simple CRC mode
    bin_data[23] = 0;
    // Step5: Write output binary
    ofstream output_bin(filename, ios::out | ios::binary);
    output_bin.write((char*)bin_data, filesize);
    output_bin.close();
    free(bin_data);
}