add_library(maat_platform_interface INTERFACE)

target_include_directories(maat_platform_interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include> # For potential future installation
)

