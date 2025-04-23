add_executable(maat_app main.cpp)

# Link the executable against the core logic and the specific platform implementation
target_link_libraries(maat_app PRIVATE maat_core)

if(WIN32)
  target_link_libraries(maat_app PRIVATE maat_platform_windows) # Link the Windows implementation
else()
    # Potentially link other platform implementations here
endif()

# Ensure the app can find headers from core and platform interface
target_link_libraries(maat_app PRIVATE maat_platform_interface) # Needed if app uses platform types directly
