#include "maat_platform_windows/windows_monitor.h"

#include <windows.h>
#include <stdexcept> // For potential future error handling

namespace maat { namespace platform {

WindowsMonitor::WindowsMonitor(HMONITOR handle) : m_handle(handle) {
     if (!m_handle) {
        // Consider throwing an exception or logging an error
    }
}

// Move constructor
WindowsMonitor::WindowsMonitor(WindowsMonitor&& other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr; // Nullify the source object's handle
}

// Move assignment operator
WindowsMonitor& WindowsMonitor::operator=(WindowsMonitor&& other) noexcept {
    if (this != &other) {
        m_handle = other.m_handle;
        other.m_handle = nullptr; // Nullify the source object's handle
    }
    return *this;
}

MonitorId WindowsMonitor::getId() const {
    // Directly cast HMONITOR to our platform-independent MonitorId (uintptr_t)
    return reinterpret_cast<MonitorId>(m_handle);
}

HMONITOR WindowsMonitor::getHandle() const {
    return m_handle;
}

Rect WindowsMonitor::getWorkArea() const {
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO); // Crucial: Set the size member

    if (GetMonitorInfoW(m_handle, &monitorInfo)) {
        const RECT& workRect = monitorInfo.rcWork; // Use rcWork for usable area
        return {
            workRect.left,
            workRect.top,
            workRect.right - workRect.left,
            workRect.bottom - workRect.top
        };
    }

    // Return an empty rect or handle error if GetMonitorInfoW fails
    return {0, 0, 0, 0};
}

} } // namespace maat::platform
