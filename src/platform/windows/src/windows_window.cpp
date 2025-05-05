#include "maat_platform_windows/windows_window.h"

#include <windows.h>
#include <stdexcept> // For potential future error handling

namespace maat { namespace platform {

WindowsWindow::WindowsWindow(HWND handle) : m_handle(handle) {
    if (!m_handle) {
        // Consider throwing an exception or logging an error
        // For now, we assume a valid handle is always passed.
    }
}

// Move constructor
WindowsWindow::WindowsWindow(WindowsWindow&& other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr; // Nullify the source object's handle
}

// Move assignment operator
WindowsWindow& WindowsWindow::operator=(WindowsWindow&& other) noexcept {
    if (this != &other) {
        m_handle = other.m_handle;
        other.m_handle = nullptr; // Nullify the source object's handle
    }
    return *this;
}


WindowId WindowsWindow::getId() const {
    // Directly cast HWND to our platform-independent WindowId (uintptr_t)
    return reinterpret_cast<WindowId>(m_handle);
}

HWND WindowsWindow::getHandle() const {
    return m_handle;
}

Rect WindowsWindow::getGeometry() const {
    RECT win_rect{};
    if (GetWindowRect(m_handle, &win_rect)) {
        return {
            win_rect.left,
            win_rect.top,
            win_rect.right - win_rect.left,
            win_rect.bottom - win_rect.top
        };
    }
    // Return an empty rect or handle error if GetWindowRect fails
    return {0, 0, 0, 0};
}

bool WindowsWindow::isManageable() const {
    if (!m_handle || !IsWindow(m_handle)) {
        return false; // Invalid handle
    }

    // 1. Must be visible
    if (!IsWindowVisible(m_handle)) {
        return false;
    }

    // 2. Must be a top-level window (not owned)
    //    GetAncestor with GA_ROOT returns the root window. If it's not the window itself, it's a child/owned.
    if (GetAncestor(m_handle, GA_ROOT) != m_handle) {
         return false;
    }

    // 3. Check standard styles
    LONG style = GetWindowLongPtr(m_handle, GWL_STYLE);
    if (style == 0 && GetLastError() != 0) {
         // Error getting style
         return false;
    }

    // Must have WS_VISIBLE (somewhat redundant with IsWindowVisible, but good check)
    // Must NOT be a child (redundant with GetAncestor)
    // Must NOT be disabled
    // Must NOT be a popup (often temporary/tool windows)
    if (!(style & WS_VISIBLE) || (style & WS_CHILD) || (style & WS_DISABLED) || (style & WS_POPUP)) {
        return false;
    }

    // 4. Check extended styles
    LONG exStyle = GetWindowLongPtr(m_handle, GWL_EXSTYLE);
     if (exStyle == 0 && GetLastError() != 0 && !(style & WS_CHILD)) {
         // Error getting exStyle, but ignore error for child windows as they might not have exStyles set
         // However, we already filtered out child windows above. Still, keep check robust.
         // Only consider error significant if it's not potentially a child window case.
         // Note: This check is complex because GetWindowLongPtr can legitimately return 0.
         // A more robust check involves SetLastError(0) before the call. For simplicity,
         // we'll assume failure if 0 is returned and GetLastError is non-zero.
         // But since we filter WS_CHILD earlier, this error case becomes less likely relevant.
         // Let's simplify: if GetLastError is set after the call, assume failure unless exStyle is actually 0.
         DWORD lastError = GetLastError();
         if(exStyle == 0 && lastError != 0) {
            return false;
         }
     }


    // Must NOT be a tool window (WS_EX_TOOLWINDOW)
    // Must NOT be non-activatable (WS_EX_NOACTIVATE)
    // Must NOT be transparent to input (WS_EX_TRANSPARENT)
    if ((exStyle & WS_EX_TOOLWINDOW) || (exStyle & WS_EX_NOACTIVATE) || (exStyle & WS_EX_TRANSPARENT)) {
        return false;
    }

    // Consider if it should be an app window.
    // Typically, windows that are not tool windows and are top-level are considered manageable.
    // WS_EX_APPWINDOW is often present on main application windows showing in the taskbar.
    // Absence of WS_EX_TOOLWINDOW is usually a strong indicator for manageability.
    // Let's stick to the exclusion logic for now as per requirements.


    // Optional: Check for zero size - although unlikely for manageable windows
    // RECT r;
    // GetWindowRect(m_handle, &r);
    // if (r.right - r.left <= 0 || r.bottom - r.top <= 0) {
    //    return false;
    // }

    // Passed all checks
    return true;
}

} } // namespace maat::platform
