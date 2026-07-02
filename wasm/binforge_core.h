#pragma once
#include <stdint.h>
#include "esp_app_format.h"
#ifdef __cplusplus
extern "C" {
#endif

	// Main analysis entry (dummy for now)
	void analyze_esp32_binary(const uint8_t* bin_data, uint32_t filesize);

	// Logging function (printf-style, implemented via JS hook later)
	void bf_log(const char* fmt, ...);

#ifdef __cplusplus
}
#endif