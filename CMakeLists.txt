cmake_minimum_required(VERSION 3.10)
project(maat VERSION 0.1.0 LANGUAGES CXX)

# Define standard output directories within the build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# For multi-configuration generators (like Visual Studio),
# set configuration-specific output directories
foreach(OUTPUT_CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUT_CONFIG} OUTPUT_CONFIG_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUT_CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${OUTPUT_CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUT_CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${OUTPUT_CONFIG})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${OUTPUT_CONFIG})
endforeach()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src/platform/interface)
add_subdirectory(src/core)
# Conditionally add platform implementation - for now, just Windows
if(WIN32)
    add_subdirectory(src/platform/windows)
else()
    message(WARNING "No platform implementation selected for this OS.")
    # Add logic here to select other platforms or error out
endif()
add_subdirectory(src/app)
