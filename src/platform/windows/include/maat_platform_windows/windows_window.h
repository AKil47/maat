#ifndef MAAT_PLATFORM_WINDOWS_WINDOWS_WINDOW_H_
#define MAAT_PLATFORM_WINDOWS_WINDOWS_WINDOW_H_

// Prevent inclusion of min/max macros which conflict with std::min/max
#define NOMINMAX
#include <windows.h>

#include "maat_platform/window.h"
#include "maat_platform/platform_types.h" // Include for Rect

namespace maat { namespace platform {

class WindowsWindow final : public Window {
public:
    explicit WindowsWindow(HWND handle);
    ~WindowsWindow() override = default;

    // Prevent copy and assignment
    WindowsWindow(const WindowsWindow&) = delete;
    WindowsWindow& operator=(const WindowsWindow&) = delete;

    // Move constructor and assignment
    WindowsWindow(WindowsWindow&& other) noexcept;
    WindowsWindow& operator=(WindowsWindow&& other) noexcept;


    WindowId getId() const override;
    Rect getGeometry() const override;
    bool isManageable() const override;

    HWND getHandle() const;

private:
    HWND m_handle;
};

} } // namespace maat::platform

#endif // MAAT_PLATFORM_WINDOWS_WINDOWS_WINDOW_H_
