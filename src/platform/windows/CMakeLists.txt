add_library(maat_platform_windows STATIC) # Or SHARED

target_sources(maat_platform_windows PRIVATE
    src/windows_platform_manager.cpp
    src/windows_window.cpp
    src/windows_monitor.cpp
)

target_include_directories(maat_platform_windows PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
   PRIVATE
    # Add private include paths if needed later
)

# Windows implementation needs the platform interface definitions
target_link_libraries(maat_platform_windows PUBLIC maat_platform_interface)

# Add Windows-specific libraries
# Note: WIN32 is always defined on Windows, even for 64-bit builds
target_link_libraries(maat_platform_windows PRIVATE User32 Gdi32) # Example libraries

# Define the minimum Windows version needed for the target
# 0x0600 corresponds to Windows Vista
target_compile_definitions(maat_platform_windows PRIVATE _WIN32_WINNT=0x0600)
