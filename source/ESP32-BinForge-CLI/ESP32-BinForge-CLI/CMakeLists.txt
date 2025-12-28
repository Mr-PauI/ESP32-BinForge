# CMakeList.txt : CMake project for ESP32-BinForge-CLI, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (ESP32-BinForge-CLI "ESP32-BinForge-CLI.cpp" "ESP32-BinForge-CLI.h" "esp_app_format.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ESP32-BinForge-CLI PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
