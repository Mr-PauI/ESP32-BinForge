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
#include <sstream>
#include <iomanip>

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hEditLog = nullptr;                        // the full-screen textbox

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void MergeROM(HWND);
uint8_t compute_esp32s3_crc(uint8_t* data, int count);
void validate_esp32_bin(const std::string filename);
void revalidate_crc_nosha256(HWND hWnd);
void analyze_file(HWND hWnd);
void analyze_bin(const std::string filename);
using namespace std;

void AppendLog(HWND hEdit, const std::string& text)
{
    int len = GetWindowTextLengthA(hEdit);
    SendMessageA(hEdit, EM_SETSEL, len, len);
    SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

void AppendLog_HEX(HWND hEditLog, uint32_t value)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << value;
        

    AppendLog(hEditLog, ss.str());
}

void ClearLog(HWND hEditLog)
{
    SetWindowText(hEditLog, L"");
}

std::streamsize get_file_size(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    return file.tellg();
}


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

    AppendLog(hEditLog, "-----------------------------------\r\n");
    AppendLog(hEditLog, "  Injecting data into esp32 binary  \r\n");
    AppendLog(hEditLog, "-----------------------------------\r\n");
    AppendLog(hEditLog, "  Locating magic pattern....");
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
        AppendLog(hEditLog, "  failed!\r\n\r\n");
        AppendLog(hEditLog, "  Operation could not be completed!\r\n");
        AppendLog(hEditLog, "  Reserved segment could not be found\r\n");
        AppendLog(hEditLog, "  Ensure you are using the correct input file\r\n");
        
        /*MessageBoxW(hWnd,
            L"Operation could not be completed\n\n"
            L"Reserved segment could not be found\n"
            L"Ensure you are using the correct input file\n",
            L"",
            MB_OK | MB_ICONWARNING
        );*/
        return;
    }
    AppendLog(hEditLog, "  complete!\r\n");
    // Perform the merge operation
    AppendLog(hEditLog, "  Merging data with binary....");
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
    AppendLog(hEditLog, "  complete!\r\n");
    // Regenerate CRC and disable the Sha256 checksum
    validate_esp32_bin(outputBIN);
    AppendLog(hEditLog, "-----------------------------\r\n");
    AppendLog(hEditLog, "  Merging binary complete! \r\n");
    //MessageBox(hWnd, L"Merge binary Complete!", L"Success!", MB_ICONEXCLAMATION | MB_OK);
    return;
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
       600, 800,
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
    case WM_SIZE:
    {
        if (hEditLog)
        {
            MoveWindow(
                hEditLog,
                0, 0,
                LOWORD(lParam),
                HIWORD(lParam),
                TRUE
            );
        }
    }
    break;
    case WM_CREATE:
    {
        hEditLog = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE |
            WS_VSCROLL | WS_HSCROLL |
            ES_LEFT | ES_MULTILINE |
            ES_AUTOVSCROLL | ES_AUTOHSCROLL |
            ES_READONLY,
            0, 0, 0, 0,
            hWnd,
            nullptr,
            hInst,
            nullptr
        );

        // Optional: fixed-width font (recommended)
        HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
        SendMessage(hEditLog, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_FILE_EXAMINE:
                analyze_file(hWnd);
                break;
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

void analyze_file(HWND hWnd)
{
    ClearLog(hEditLog);
    string inputBIN = "";
    inputBIN = ShowOpenFileDialogBIN(hWnd);
    if (inputBIN != "")
    {
        analyze_bin(inputBIN);
    }
}

void revalidate_crc_nosha256(HWND hWnd)
{
    ClearLog(hEditLog);
    string inputBIN = "";
    inputBIN = ShowOpenFileDialogBIN(hWnd);
    if (inputBIN != "")
    {
        validate_esp32_bin(inputBIN);
        // Let the log notify the user
        //MessageBox(hWnd, L"File CRC regenerated, SHA256 hash disabled", L"Done!", MB_ICONEXCLAMATION | MB_OK);
    }
}

void validate_esp32_bin(const std::string filename)
{    
    AppendLog(hEditLog, "-----------------------------\r\n");
    AppendLog(hEditLog, "  Revalidating esp32 binary  \r\n");
    AppendLog(hEditLog, "-----------------------------\r\n");
    // Regenerate CRC and disable the Sha256 checksum
    // Step1: Read the entire outBin into memory
    uint8_t stats_images = 0;
    uint8_t stats_errors = 0;
    uint8_t stats_sha256_disabledCount = 0;
    uint8_t stats_crc_regenCount = 0;
    uint8_t* bin_data;
    uint8_t crc_value;
    // Get the file size, there is a faster way to do this, but for now this works
    uint32_t filesize=(uint32_t)get_file_size(filename);
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
            AppendLog(hEditLog,"Image found at offset 0x" + std::to_string(file_position) + "\r\n");
            esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(bin_data + file_position);
            // Log SPI mode
            AppendLog(hEditLog, "   Flash Mode: ");   
            switch (hdr->spi_mode)
            {
            case 0:
                AppendLog(hEditLog, "QIO\r\n");
                break;
            case 1:
                AppendLog(hEditLog, "QOUT\r\n");
                break;
            case 2:
                AppendLog(hEditLog, "DIO\r\n");
                break;
            case 3:
                AppendLog(hEditLog, "DOUT\r\n");
                break;
            case 4:
                AppendLog(hEditLog, "FAST READ\r\n");
                break;
            case 5:
                AppendLog(hEditLog, "SLOW READ\r\n");
                break;
            default:
                AppendLog(hEditLog, "Unknown\r\n");
                break;
            }
            AppendLog(hEditLog, "      Chip ID: ");
            switch (hdr->chip_id)
            {
            case ESP_CHIP_ID_ESP32:
                AppendLog(hEditLog, "ESP32\r\n");
                break;
            case ESP_CHIP_ID_ESP32S2:
                AppendLog(hEditLog, "ESP32-S2\r\n");
                break;
            case ESP_CHIP_ID_ESP32C3:
                AppendLog(hEditLog, "ESP32-C3\r\n");
                break;
            case ESP_CHIP_ID_ESP32S3:
                AppendLog(hEditLog, "ESP32-S3\r\n");
                break;
            case ESP_CHIP_ID_ESP32C2:
                AppendLog(hEditLog, "ESP32-C2\r\n");
                break;
            case ESP_CHIP_ID_ESP32C6:
                AppendLog(hEditLog, "ESP32-C6\r\n");
                break;
            case ESP_CHIP_ID_ESP32H2:
                AppendLog(hEditLog, "ESP32-H2\r\n");
                break;
            case ESP_CHIP_ID_ESP32P4:
                AppendLog(hEditLog, "ESP32-P4\r\n");
                break;
            case ESP_CHIP_ID_ESP32C5:
                AppendLog(hEditLog, "ESP32-C5\r\n");
                break;
            case ESP_CHIP_ID_ESP32C61:
                AppendLog(hEditLog, "ESP32-C61\r\n");
                break;
            case ESP_CHIP_ID_ESP32H21:
                AppendLog(hEditLog, "ESP32-H21\r\n");
                break;
            case ESP_CHIP_ID_ESP32H4:
                AppendLog(hEditLog, "ESP32-H4\r\n");
                break;
            case ESP_CHIP_ID_ESP32S31:
                AppendLog(hEditLog, "ESP32-S31\r\n");
                break;
            default:
                AppendLog(hEditLog, "Unknown\r\n");
                break;
            }
            AppendLog(hEditLog, "Entry Address: ");
            AppendLog_HEX(hEditLog, hdr->entry_addr);
            AppendLog(hEditLog, "\r\n");
            
            stats_images++;
            image_header = file_position;
            file_position += 24; // position us at the beginning of the first segment
            uint8_t segment_count = bin_data[image_header + 1]; // 2nd byte contains number of segments
            // Log number of segments
            AppendLog(hEditLog, "     Segments: " + std::to_string(segment_count) + "\r\n");
            AppendLog(hEditLog, "-----------------------\r\n");
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
                AppendLog(hEditLog, " Segment #" + std::to_string(j) + "\r\n     Load Addr: ");
                AppendLog_HEX(hEditLog, segment_addr); // this would be its address in virtual space
                AppendLog(hEditLog, "\r\n   File Offset: ");
                AppendLog_HEX(hEditLog, file_position);
                AppendLog(hEditLog, "\r\n          Size: " + std::to_string(segment_size) + " bytes\r\n");
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
            AppendLog(hEditLog, "Image original CRC:" + std::to_string(old_crc) + "\r\n");
            AppendLog(hEditLog, "Image computed CRC:" + std::to_string(crc_value) + "\r\n");
            // Step3: Update the CRC value, disable sha256 ; forced for now - to avoid sha256 errors on duplicate crcs
            //if (crc_value != bin_data[crc_location])
            {
                if (crc_location > filesize)
                {
                    stats_errors++;
                    crc_location = filesize - 33;// just a temporary thing until the crc locating is correct
                    AppendLog(hEditLog, " ERROR!! Expectd CRC not located within file\r\n");
                }
                else
                {
                    stats_crc_regenCount++;
                    bin_data[crc_location] = crc_value;
                }
                // Step4: Change checksum to simple CRC mode
                if (bin_data[image_header + 23])
                {
                    AppendLog(hEditLog, "   Image has has sha256\r\n");
                    bin_data[image_header + 23] = 0; // disables sha256
                    file_position = crc_location + 32 + 1;
                    AppendLog(hEditLog, "   Image sha256 has been disabled\r\n");
                    stats_sha256_disabledCount++;
                }
                else
                {
                    AppendLog(hEditLog, "   Image had no sha256 to disable\r\n");
                    file_position = crc_location + 1;
                }                
            }
        }
        else
            file_position += 16;// image headers are always 16-byte aligned
    }
    // Step5: Write output binary
    ofstream output_bin(filename, ios::out | ios::binary);
    AppendLog(hEditLog, "-----------------------------\r\n");
    AppendLog(hEditLog, "  Revalidation completed: " + std::to_string(filesize) + " bytes written\r\n");
    output_bin.write((char*)bin_data, filesize);
    output_bin.close();
    free(bin_data);
}

void analyze_bin(const std::string filename)
{
    AppendLog(hEditLog, "-----------------------------\r\n");
    AppendLog(hEditLog, "  Analyze esp32 binary  \r\n");
    
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
            AppendLog(hEditLog, "-----------------------------\r\n");
            AppendLog(hEditLog, "Image found at offset 0x" + std::to_string(file_position) + "\r\n");
            esp_image_header_t* hdr = reinterpret_cast<esp_image_header_t*>(bin_data + file_position);
            // Log SPI mode
            AppendLog(hEditLog, "   Flash Mode: ");
            switch (hdr->spi_mode)
            {
            case 0:
                AppendLog(hEditLog, "QIO\r\n");
                break;
            case 1:
                AppendLog(hEditLog, "QOUT\r\n");
                break;
            case 2:
                AppendLog(hEditLog, "DIO\r\n");
                break;
            case 3:
                AppendLog(hEditLog, "DOUT\r\n");
                break;
            case 4:
                AppendLog(hEditLog, "FAST READ\r\n");
                break;
            case 5:
                AppendLog(hEditLog, "SLOW READ\r\n");
                break;
            default:
                AppendLog(hEditLog, "Unknown\r\n");
                break;
            }
            AppendLog(hEditLog, "      Chip ID: ");
            switch (hdr->chip_id)
            {
            case ESP_CHIP_ID_ESP32:
                AppendLog(hEditLog, "ESP32\r\n");
                break;
            case ESP_CHIP_ID_ESP32S2:
                AppendLog(hEditLog, "ESP32-S2\r\n");
                break;
            case ESP_CHIP_ID_ESP32C3:
                AppendLog(hEditLog, "ESP32-C3\r\n");
                break;
            case ESP_CHIP_ID_ESP32S3:
                AppendLog(hEditLog, "ESP32-S3\r\n");
                break;
            case ESP_CHIP_ID_ESP32C2:
                AppendLog(hEditLog, "ESP32-C2\r\n");
                break;
            case ESP_CHIP_ID_ESP32C6:
                AppendLog(hEditLog, "ESP32-C6\r\n");
                break;
            case ESP_CHIP_ID_ESP32H2:
                AppendLog(hEditLog, "ESP32-H2\r\n");
                break;
            case ESP_CHIP_ID_ESP32P4:
                AppendLog(hEditLog, "ESP32-P4\r\n");
                break;
            case ESP_CHIP_ID_ESP32C5:
                AppendLog(hEditLog, "ESP32-C5\r\n");
                break;
            case ESP_CHIP_ID_ESP32C61:
                AppendLog(hEditLog, "ESP32-C61\r\n");
                break;
            case ESP_CHIP_ID_ESP32H21:
                AppendLog(hEditLog, "ESP32-H21\r\n");
                break;
            case ESP_CHIP_ID_ESP32H4:
                AppendLog(hEditLog, "ESP32-H4\r\n");
                break;
            case ESP_CHIP_ID_ESP32S31:
                AppendLog(hEditLog, "ESP32-S31\r\n");
                break;
            default:
                AppendLog(hEditLog, "Unknown\r\n");
                break;
            }
            AppendLog(hEditLog, "Entry Address: ");
            AppendLog_HEX(hEditLog, hdr->entry_addr);
            AppendLog(hEditLog, "\r\n");

            stats_images++;
            image_header = file_position;
            file_position += 24; // position us at the beginning of the first segment
            uint8_t segment_count = bin_data[image_header + 1]; // 2nd byte contains number of segments
            // Log number of segments
            AppendLog(hEditLog, "     Segments: " + std::to_string(segment_count) + "\r\n");
            AppendLog(hEditLog, "-----------------------\r\n");
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
                AppendLog(hEditLog, " Segment #" + std::to_string(j) + "\r\n     Load Addr: ");
                AppendLog_HEX(hEditLog, segment_addr); // this would be its address in virtual space
                AppendLog(hEditLog, "\r\n   File Offset: ");
                AppendLog_HEX(hEditLog, file_position);
                AppendLog(hEditLog, "\r\n          Size: " + std::to_string(segment_size) + " bytes\r\n");
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
            AppendLog(hEditLog, "Image original CRC:" + std::to_string(old_crc) + "\r\n");
            AppendLog(hEditLog, "Image computed CRC:" + std::to_string(crc_value) + "\r\n");
            // Step3: Update the CRC value, forced for now - to avoid sha256 errors on duplicate crcs
            if (crc_value != bin_data[crc_location])
                AppendLog(hEditLog, " **** WARNING **** CRC MISMATCH\r\n");

            
            if (crc_location > filesize)
            {
                stats_errors++;
                crc_location = filesize - 33;// just a temporary thing until the crc locating is correct
                AppendLog(hEditLog, " ERROR!! Expectd CRC not located within file. Segment header faulty or file incomplete.\r\n");
            }
            else
            {
                stats_crc_regenCount++;
                bin_data[crc_location] = crc_value;
            }
            // Step4: Change checksum to simple CRC mode
            if (bin_data[image_header + 23])
            {
                AppendLog(hEditLog, "   Image has has sha256\r\n");
                file_position = crc_location + 32 + 1;
            }
            else
            {
                AppendLog(hEditLog, "   Image has no sha256\r\n");
                file_position = crc_location + 1;
            }
            
        }
        else
            file_position += 16;// image headers are always 16-byte aligned
    }
    // End of file analysis
    AppendLog(hEditLog, "-----------------------------\r\n");
    AppendLog(hEditLog, "  Total binary file size: " + std::to_string(filesize) + " bytes \r\n");
    free(bin_data);
}