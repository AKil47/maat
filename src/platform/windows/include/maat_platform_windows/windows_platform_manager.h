#ifndef MAAT_PLATFORM_WINDOWS_WINDOWS_PLATFORM_MANAGER_H_
#define MAAT_PLATFORM_WINDOWS_WINDOWS_PLATFORM_MANAGER_H_

// Prevent inclusion of min/max macros
#define NOMINMAX

// Define Windows version to Vista or higher to get access to EVENT_SYSTEM_DISPLAYCHANGE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <windows.h>

#include <vector>
#include <map>
#include <utility>
#include <functional>
#include <memory>
#include <mutex> // Added for thread safety

#include "maat_platform/platform_manager.h"
#include "maat_platform/platform_types.h"

// Forward declarations instead of including full headers here
namespace maat { namespace platform {
class WindowsWindow;
class WindowsMonitor;
} } // namespace maat::platform

namespace maat::platform {

class WindowsPlatformManager final : public PlatformManager {
public:
    WindowsPlatformManager();
    ~WindowsPlatformManager() override;

    // Prevent copy and assignment
    WindowsPlatformManager(const WindowsPlatformManager&) = delete;
    WindowsPlatformManager& operator=(const WindowsPlatformManager&) = delete;

    // --- PlatformManager Interface Overrides ---

    void applyWindowGeometries(const std::vector<std::pair<WindowId, Rect>>& updates) override;
    std::vector<Monitor*> enumerateMonitors() override;
    std::vector<Window*> enumerateInitialWindows() override;

    void setWindowCreatedCallback(std::function<void(Window*)> cb) override;
    void setWindowDestroyedCallback(std::function<void(WindowId)> cb) override;
    void setWindowMonitorChangedCallback(std::function<void(WindowId, MonitorId)> cb) override;
    void setMonitorLayoutChangedCallback(std::function<void()> cb) override;

    void releaseWindowTracking(WindowId id) override;

    void startEventLoop() override;
    void stopEventLoop() override;

private:
    friend LRESULT CALLBACK HelperWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL CALLBACK StaticMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    friend BOOL CALLBACK StaticWindowEnumProc(HWND hwnd, LPARAM lParam);
    friend void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

    // --- Internal Helper Methods ---
    void clearMonitors();
    void clearWindows();
    void registerEventHooks();
    void unregisterEventHooks();

    // Helper window management
    bool registerHelperWindowClass();
    bool createHelperWindow();
    void destroyHelperWindow();

    // --- Event Handling ---
    // Non-static member function to handle events forwarded by the static proc
    void HandleWindowEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

    // --- Win32 Callback Functions (Static) ---
    static BOOL CALLBACK StaticMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    static BOOL CALLBACK StaticWindowEnumProc(HWND hwnd, LPARAM lParam);
    static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
    static LRESULT CALLBACK HelperWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // --- Member Variables ---

    // Ownership maps: The manager owns these objects.
    std::map<MonitorId, WindowsMonitor*> m_monitors;
    std::map<WindowId, WindowsWindow*> m_windows;

    // Callback storage
    std::function<void(Window*)> m_windowCreatedCallback;
    std::function<void(WindowId)> m_windowDestroyedCallback;
    std::function<void(WindowId, MonitorId)> m_windowMonitorChangedCallback;
    std::function<void()> m_monitorLayoutChangedCallback;

    // Event loop control (basic example)
    DWORD m_eventLoopThreadId = 0;
    bool m_stopEventLoop = false;

    // WinEventHook handles
    HWINEVENTHOOK m_hHookCreate = nullptr;
    HWINEVENTHOOK m_hHookDestroy = nullptr;
    HWINEVENTHOOK m_hHookMoveSize = nullptr;
    HWINEVENTHOOK m_hHookDisplayChange = nullptr; // Changed from MonitorChange based on thinking process

    // Helper window handle
    HWND m_hHelperWindow = nullptr;

    // Static map to associate hook handles with instances
    // Protected by s_hookMapMutex
    static std::map<HWINEVENTHOOK, WindowsPlatformManager*> s_hookMap;
    static std::mutex s_hookMapMutex;
    static const wchar_t* const kHelperWindowClassName;
};

} // namespace maat::platform

#endif // MAAT_PLATFORM_WINDOWS_WINDOWS_PLATFORM_MANAGER_H_
