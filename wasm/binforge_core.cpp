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
void analyze_esp32_binary(const uint8_t* bin_data, uint32_t filesize)
{
    bf_log("BinForge Analysis Started\n");
    bf_log("File size: %u bytes\n", filesize);

    if (!bin_data || filesize == 0)
    {
        bf_log("ERROR: Invalid input buffer\n");
        return;
    }

    bf_log("First byte: 0x%02X\n", bin_data[0]);
    bf_log("Analysis complete (dummy stage)\n");
}