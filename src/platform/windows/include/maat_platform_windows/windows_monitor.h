#ifndef MAAT_PLATFORM_WINDOWS_WINDOWS_MONITOR_H_
#define MAAT_PLATFORM_WINDOWS_WINDOWS_MONITOR_H_

// Prevent inclusion of min/max macros
#define NOMINMAX
#include <windows.h>

#include "maat_platform/monitor.h"
#include "maat_platform/platform_types.h" // Include for Rect

namespace maat { namespace platform {

class WindowsMonitor final : public Monitor {
public:
    explicit WindowsMonitor(HMONITOR handle);
    ~WindowsMonitor() override = default;

    // Prevent copy and assignment
    WindowsMonitor(const WindowsMonitor&) = delete;
    WindowsMonitor& operator=(const WindowsMonitor&) = delete;

    // Move constructor and assignment
    WindowsMonitor(WindowsMonitor&& other) noexcept;
    WindowsMonitor& operator=(WindowsMonitor&& other) noexcept;

    MonitorId getId() const override;
    Rect getWorkArea() const override;

    HMONITOR getHandle() const;

private:
    HMONITOR m_handle;
};

} } // namespace maat::platform

#endif // MAAT_PLATFORM_WINDOWS_WINDOWS_MONITOR_H_
