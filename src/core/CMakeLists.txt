add_library(maat_core STATIC) # Or SHARED if preferred

target_sources(maat_core PRIVATE
    src/core_manager.cpp
    src/layout_tree.cpp
    src/maat_mediator.cpp
)

target_include_directories(maat_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
   PRIVATE
    # Add private include paths if needed later
)

# Core depends on the platform interface definitions
target_link_libraries(maat_core PUBLIC maat_platform_interface)
