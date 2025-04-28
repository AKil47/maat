#include "maat_platform_windows/windows_platform_manager.h"

#include "maat_platform_windows/windows_window.h"
#include "maat_platform_windows/windows_monitor.h"

// Ensure NOMINMAX is defined if not included via header
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Define Windows version (still good practice)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <windows.h>
#include <winuser.h>

// Define EVENT_SYSTEM_DISPLAYCHANGE if not defined
#ifndef EVENT_SYSTEM_DISPLAYCHANGE
#define EVENT_SYSTEM_DISPLAYCHANGE 0x000D
#endif

#include <vector>
#include <map>
#include <utility>
#include <algorithm> // For std::transform if needed later
#include <mutex> // Include mutex header
#include <iostream> // For potential error logging

namespace maat::platform {

// --- Static Member Initialization ---
std::map<HWINEVENTHOOK, WindowsPlatformManager*> WindowsPlatformManager::s_hookMap;
std::mutex WindowsPlatformManager::s_hookMapMutex;
const wchar_t* const WindowsPlatformManager::kHelperWindowClassName = L"MaatPlatformHelperWindowClass";

// --- Constructor & Destructor ---

WindowsPlatformManager::WindowsPlatformManager() :
    m_eventLoopThreadId(0),
    m_stopEventLoop(false),
    m_hHookCreate(nullptr),
    m_hHookDestroy(nullptr),
    m_hHookMoveSize(nullptr),
    m_hHelperWindow(nullptr)
{}

WindowsPlatformManager::~WindowsPlatformManager() {
    // Destroy helper window *before* unregistering class (if applicable)
    // Message loop should be stopped before destructor is called.
    destroyHelperWindow();
    unregisterEventHooks(); // Unhook remaining hooks
    // Potentially unregister window class if needed, but often not necessary for message-only windows

    clearWindows();
    clearMonitors();
}

// --- Private Helper Methods ---

void WindowsPlatformManager::clearMonitors() {
    for (auto const& [id, monitor_ptr] : m_monitors) {
        delete monitor_ptr;
    }
    m_monitors.clear();
}

void WindowsPlatformManager::clearWindows() {
    for (auto const& [id, window_ptr] : m_windows) {
        delete window_ptr;
    }
    m_windows.clear();
}

// --- Win32 Callback Implementations (Static Enums) ---

BOOL CALLBACK WindowsPlatformManager::StaticMonitorEnumProc(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM dwData) {
    auto* self = reinterpret_cast<WindowsPlatformManager*>(dwData);
    if (!self) return FALSE;
    MonitorId id = reinterpret_cast<MonitorId>(hMonitor);
    if (self->m_monitors.find(id) == self->m_monitors.end()) {
        self->m_monitors[id] = new WindowsMonitor(hMonitor);
    }
    return TRUE;
}

BOOL CALLBACK WindowsPlatformManager::StaticWindowEnumProc(HWND hwnd, LPARAM lParam) {
    auto* self = reinterpret_cast<WindowsPlatformManager*>(lParam);
    if (!self) return FALSE;
    WindowId id = reinterpret_cast<WindowId>(hwnd);
    if (self->m_windows.find(id) == self->m_windows.end()) {
        // Simple creation, filtering happens later or during event handling
        self->m_windows[id] = new WindowsWindow(hwnd);
    }
    return TRUE;
}

// --- Static WinEventProc ---

void CALLBACK WindowsPlatformManager::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    WindowsPlatformManager* instance = nullptr;
    {
        std::lock_guard<std::mutex> lock(s_hookMapMutex);
        auto it = s_hookMap.find(hWinEventHook);
        if (it != s_hookMap.end()) {
            instance = it->second;
        }
    } // Mutex released here

    if (instance) {
        // Forward the event to the non-static member function
        instance->HandleWindowEvent(hWinEventHook, event, hwnd, idObject, idChild, dwEventThread, dwmsEventTime);
    }
}

// --- Static Helper Window Procedure ---
LRESULT CALLBACK WindowsPlatformManager::HelperWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowsPlatformManager* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        // Associate the PlatformManager instance with the window handle
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<WindowsPlatformManager*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        // Retrieve the PlatformManager instance pointer
        pThis = reinterpret_cast<WindowsPlatformManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (uMsg) {
            case WM_DISPLAYCHANGE:
                // Display settings changed, notify the manager
                if (pThis->m_monitorLayoutChangedCallback) {
                    pThis->m_monitorLayoutChangedCallback();
                }
                return 0; // Indicate message was handled

            // Handle other messages if needed (e.g., WM_DESTROY)
            case WM_DESTROY:
                // Clean up the association when the window is destroyed
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
                // pThis->m_hHelperWindow = nullptr; // Mark handle as invalid in manager
                break; // Let DefWindowProc handle the rest

        }
    }

    // Pass unhandled messages to the default window procedure
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// --- Non-Static Event Handler ---

void WindowsPlatformManager::HandleWindowEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    // Filter out events that are not for top-level windows
    if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF || !hwnd) {
         // For DISPLAYCHANGE, hwnd might be NULL. Handle specific cases.
         if(event != EVENT_SYSTEM_DISPLAYCHANGE) {
            return;
         }
    }

    WindowId windowId = reinterpret_cast<WindowId>(hwnd);

    switch (event) {
        case EVENT_OBJECT_CREATE: {
            // Check if window handle is valid and not already tracked
            if (IsWindow(hwnd)) { // Check validity here
                 // Avoid double-adding if EnumWindows race condition or similar occurs
                 if (m_windows.find(windowId) == m_windows.end()) {
                    auto* tempWindow = new WindowsWindow(hwnd);
                    if (tempWindow->isManageable()) {
                        m_windows[windowId] = tempWindow; // Add to tracked windows
                        if (m_windowCreatedCallback) {
                            m_windowCreatedCallback(tempWindow); // Notify core
                        }
                    } else {
                        delete tempWindow; // Not manageable, discard immediately
                    }
                 }
            }
            break;
        }

        case EVENT_OBJECT_DESTROY: {
             // Check if we were tracking this window
            auto it = m_windows.find(windowId);
            if (it != m_windows.end()) {
                // We were tracking it. Notify the core logic.
                // The core logic MUST call releaseWindowTracking later.
                if (m_windowDestroyedCallback) {
                    m_windowDestroyedCallback(windowId);
                }
                // DO NOT delete it->second here. Deletion happens in releaseWindowTracking.
                // DO NOT remove from map here, wait for releaseWindowTracking.
            }
            break;
        }

        case EVENT_SYSTEM_MOVESIZEEND: {
            auto it = m_windows.find(windowId);
            if (it != m_windows.end()) {
                 // Check if window is still valid before getting monitor
                 if (IsWindow(hwnd)) {
                    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
                    if (hMonitor && m_windowMonitorChangedCallback) {
                         MonitorId monitorId = reinterpret_cast<MonitorId>(hMonitor);
                         // TODO: Potentially compare with a stored previous monitor ID
                         //       to avoid redundant callbacks if the monitor didn't actually change.
                         m_windowMonitorChangedCallback(windowId, monitorId);
                    }
                 }
            }
            break;
        }

        // Add other event cases if needed (e.g., EVENT_OBJECT_NAMECHANGE)
    }
}

// --- PlatformManager Interface Implementation ---

std::vector<Monitor*> WindowsPlatformManager::enumerateMonitors() {
    clearMonitors();
    EnumDisplayMonitors(NULL, NULL, StaticMonitorEnumProc, reinterpret_cast<LPARAM>(this));
    std::vector<Monitor*> result;
    result.reserve(m_monitors.size());
    for (auto const& [id, monitor_ptr] : m_monitors) {
        result.push_back(monitor_ptr);
    }
    return result;
}

std::vector<Window*> WindowsPlatformManager::enumerateInitialWindows() {
    // Clear potentially stale window list from previous runs or calls
    clearWindows();
    EnumWindows(StaticWindowEnumProc, reinterpret_cast<LPARAM>(this));
    std::vector<Window*> result;
    result.reserve(m_windows.size());
    for (auto const& [id, window_ptr] : m_windows) {
        // Note: We return all enumerated windows here.
        // The core logic is responsible for filtering using isManageable().
        result.push_back(window_ptr);
    }
    return result;
}

void WindowsPlatformManager::applyWindowGeometries(const std::vector<std::pair<WindowId, Rect>>& updates) {
    if (updates.empty()) return;

    HDWP hdwp = BeginDeferWindowPos(static_cast<int>(updates.size()));
    if (!hdwp) return;

    HDWP currentHdwp = hdwp; // Use a temporary variable to check DeferWindowPos result

    for (const auto& update : updates) {
        HWND hwnd = reinterpret_cast<HWND>(update.first);
        const Rect& rect = update.second;
        UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS;

        // Check if the window still exists before trying to move it
        if (!IsWindow(hwnd)) continue;

        HDWP tempHdwp = DeferWindowPos(currentHdwp, hwnd, NULL, rect.x, rect.y, rect.width, rect.height, flags);
        if (!tempHdwp) {
            // Failed to defer this window, log error (GetLastError())
            // Abort the entire batch? Or continue? For now, abort.
            // We don't call EndDeferWindowPos if any Defer fails, as the HDWP becomes invalid.
             // Maybe log error: std::cerr << "DeferWindowPos failed for HWND " << hwnd << " Error: " << GetLastError() << std::endl;
             // Important: MSDN states BeginDeferWindowPos handle should NOT be used further if DeferWindowPos fails.
             // No need to call EndDeferWindowPos in this failure case.
            return; // Exit the function, aborting the batch.
        }
        currentHdwp = tempHdwp; // Update the handle for the next call
    }

    // Only call EndDeferWindowPos if all DeferWindowPos calls succeeded
    EndDeferWindowPos(currentHdwp);
}

void WindowsPlatformManager::setWindowCreatedCallback(std::function<void(Window*)> cb) {
    m_windowCreatedCallback = std::move(cb);
}

void WindowsPlatformManager::setWindowDestroyedCallback(std::function<void(WindowId)> cb) {
    m_windowDestroyedCallback = std::move(cb);
}

void WindowsPlatformManager::setWindowMonitorChangedCallback(std::function<void(WindowId, MonitorId)> cb) {
    m_windowMonitorChangedCallback = std::move(cb);
}

void WindowsPlatformManager::setMonitorLayoutChangedCallback(std::function<void()> cb) {
    m_monitorLayoutChangedCallback = std::move(cb);
}

void WindowsPlatformManager::releaseWindowTracking(WindowId id) {
    auto it = m_windows.find(id);
    if (it != m_windows.end()) {
        delete it->second;    // Delete the owned WindowsWindow object
        m_windows.erase(it); // Remove the entry from the map
    }
}

// --- Helper Window Management ---
bool WindowsPlatformManager::registerHelperWindowClass() {
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) }; // Use W version for class name
    wc.lpfnWndProc   = HelperWndProc;
    wc.hInstance     = GetModuleHandle(NULL); // Get instance handle
    wc.lpszClassName = kHelperWindowClassName;

    if (!RegisterClassExW(&wc)) {
        // Log error GetLastError()
        if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
             return true; // Class already registered, that's okay
        }
        return false;
    }
    return true;
}

bool WindowsPlatformManager::createHelperWindow() {
     if (!m_hHelperWindow) { // Only create if it doesn't exist
         // Pass 'this' pointer so HelperWndProc can associate it
        m_hHelperWindow = CreateWindowExW(
            0,                              // Optional window styles.
            kHelperWindowClassName,         // Window class
            L"Maat Helper Window",          // Window text (Not visible)
            0,                              // Window style (Not visible)
            0, 0, 0, 0,                     // Size and position (Not visible)
            HWND_MESSAGE,                   // Parent window: Use HWND_MESSAGE for message-only
            NULL,                           // Menu
            GetModuleHandle(NULL),          // Instance handle
            this                            // Additional application data (pass 'this')
        );

        if (!m_hHelperWindow) {
             // Log error GetLastError()
             return false;
         }
     }
     return true;
}

void WindowsPlatformManager::destroyHelperWindow() {
    if (m_hHelperWindow) {
        DestroyWindow(m_hHelperWindow);
        m_hHelperWindow = nullptr;
    }
    // Optionally unregister class here if needed, but often not necessary
    // UnregisterClassW(kHelperWindowClassName, GetModuleHandle(NULL));
}

// --- Hook Registration ---
void WindowsPlatformManager::registerEventHooks() {
    unregisterEventHooks(); // Clear existing hooks first

    DWORD targetThreadId = 0;
    DWORD targetProcessId = 0;
    UINT flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;

    m_hHookCreate = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL, WinEventProc, targetProcessId, targetThreadId, flags);
    m_hHookDestroy = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_DESTROY, NULL, WinEventProc, targetProcessId, targetThreadId, flags);
    m_hHookMoveSize = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, NULL, WinEventProc, targetProcessId, targetThreadId, flags);

    // Update Static Map
    std::lock_guard<std::mutex> lock(s_hookMapMutex);
    if (m_hHookCreate) s_hookMap[m_hHookCreate] = this; else { /* Log */ }
    if (m_hHookDestroy) s_hookMap[m_hHookDestroy] = this; else { /* Log */ }
    if (m_hHookMoveSize) s_hookMap[m_hHookMoveSize] = this; else { /* Log */ }
}

void WindowsPlatformManager::unregisterEventHooks() {
    // Store handles locally before clearing members
    HWINEVENTHOOK hooksToUnregister[] = {
        m_hHookCreate, m_hHookDestroy, m_hHookMoveSize
    };

    // Clear member handles immediately
    m_hHookCreate = m_hHookDestroy = m_hHookMoveSize = nullptr;

    // --- Remove from Static Map & Unhook (Mutex Protected Map Access) ---
    { // Scope for mutex lock
         std::lock_guard<std::mutex> lock(s_hookMapMutex);
         for (HWINEVENTHOOK hHook : hooksToUnregister) {
            if (hHook) {
                s_hookMap.erase(hHook); // Remove from map first
            }
         }
    } // Mutex released

    // Now unhook outside the lock
    for (HWINEVENTHOOK hHook : hooksToUnregister) {
        if (hHook) {
            UnhookWinEvent(hHook);
        }
    }
}

// --- Event Loop ---
void WindowsPlatformManager::startEventLoop() {
    m_eventLoopThreadId = GetCurrentThreadId();
    m_stopEventLoop = false;

    // Register class and create helper window *before* registering hooks
    // that might rely on the window existing (though not strictly necessary here)
    if (!registerHelperWindowClass() || !createHelperWindow()) {
         // Failed to set up helper window, cannot proceed reliably
         // Log error
         m_eventLoopThreadId = 0;
         return;
    }

    registerEventHooks(); // Register hooks after helper window is ready

    // Standard Windows message loop - this will now process messages for the helper window too
    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) { // Use NULL hwnd to get messages for any window on this thread
        if (bRet == -1) {
            // Log error GetLastError()
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg); // This dispatches to HelperWndProc when msg is for m_hHelperWindow
        }
    }

    // Loop exited
    unregisterEventHooks();
    destroyHelperWindow(); // Clean up helper window
    m_eventLoopThreadId = 0;
}

void WindowsPlatformManager::stopEventLoop() {
    if (m_eventLoopThreadId != 0) {
        PostThreadMessage(m_eventLoopThreadId, WM_QUIT, 0, 0);
        // Optionally wait for the thread to finish if startEventLoop runs on a separate thread
    }
    m_stopEventLoop = true; // Flag might still be useful internally
}

} // namespace maat::platform
