#include <iostream>
#include <vector>
#include <csignal>
#include <memory> // For std::unique_ptr
#include <functional> // For std::function

// Maat Platform headers
#include "maat_platform_windows/windows_platform_manager.h" // Specific implementation
#include "maat_platform/window.h"
#include "maat_platform/monitor.h"
#include "maat_platform/platform_types.h"

// Global pointer to the platform manager for the signal handler
// Using unique_ptr ensures cleanup.
std::unique_ptr<maat::platform::WindowsPlatformManager> g_platformManager = nullptr;

// Signal handler function
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    if (g_platformManager) { // Check if the unique_ptr holds a valid pointer
        std::cout << "Requesting event loop stop..." << std::endl;
        g_platformManager->stopEventLoop(); // Safe to call on the unique_ptr directly
    } else {
        std::cout << "Platform manager not initialized, exiting." << std::endl;
        // Force exit if manager isn't there to stop loop
        exit(signum);
    }
}

// Helper function to print Rect
void printRect(const maat::platform::Rect& rect) {
    std::cout << "{x=" << rect.x << ", y=" << rect.y
              << ", w=" << rect.width << ", h=" << rect.height << "}";
}


int main(int /*argc*/, char* /*argv*/[]) {
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signalHandler);

    std::cout << "Initializing WindowsPlatformManager..." << std::endl;
    // Create the manager and assign it to the global unique_ptr
    g_platformManager = std::make_unique<maat::platform::WindowsPlatformManager>();

    // --- List Initial Windows ---
    std::cout << "Enumerating initial windows..." << std::endl;
    std::vector<maat::platform::Window*> initialWindows = g_platformManager->enumerateInitialWindows();
    std::cout << "Found " << initialWindows.size() << " total initial windows. Checking manageability..." << std::endl;

    for (maat::platform::Window* window : initialWindows) {
        // Check pointer validity just in case (though enumerate should return valid ones)
        if (window && window->isManageable()) {
            maat::platform::Rect geom = window->getGeometry();
            std::cout << "  Initial Manageable Window: ID=" << window->getId() << " Geometry=";
            printRect(geom);
            std::cout << std::endl;
        }
        // Note: The PlatformManager owns the Window objects created during enumeration.
        // Cleanup is handled by the PlatformManager's destructor.
    }
    std::cout << "Finished enumerating initial manageable windows." << std::endl;

    // --- Register Event Callbacks ---
    std::cout << "Registering event callbacks..." << std::endl;

    g_platformManager->setWindowCreatedCallback([](maat::platform::Window* window) {
        // Always check pointer validity in callbacks
        if (window && window->isManageable()) {
             maat::platform::Rect geom = window->getGeometry();
             std::cout << "[Callback] Window Created: ID=" << window->getId() << " Geometry=";
             printRect(geom);
             std::cout << std::endl;
        }
        // We don't delete the 'window' pointer here; the PlatformManager owns it.
    });

    g_platformManager->setWindowDestroyedCallback([](maat::platform::WindowId id) {
        std::cout << "[Callback] Window Destroyed: ID=" << id << std::endl;
        // The PlatformManager should call releaseWindowTracking internally, which deletes the object.
    });

    g_platformManager->setWindowMonitorChangedCallback([](maat::platform::WindowId windowId, maat::platform::MonitorId monitorId) {
        std::cout << "[Callback] Window ID=" << windowId << " moved to Monitor ID=" << monitorId << std::endl;
    });

    g_platformManager->setMonitorLayoutChangedCallback([]() {
        std::cout << "[Callback] Monitor layout changed." << std::endl;
        // Optional: You could re-enumerate monitors here if needed for application state.
        // Example:
        // if (g_platformManager) {
        //     std::cout << "  Re-enumerating monitors..." << std::endl;
        //     auto monitors = g_platformManager->enumerateMonitors(); // PM handles cleanup
        //     std::cout << "  Current monitor count: " << monitors.size() << std::endl;
        //     for(const auto* monitor : monitors) {
        //          if(monitor) {
        //              std::cout << "    Monitor ID: " << monitor->getId() << " WorkArea: ";
        //              printRect(monitor->getWorkArea());
        //              std::cout << std::endl;
        //          }
        //     }
        // }
    });

    std::cout << "Callbacks registered." << std::endl;

    // --- Run Event Loop ---
    std::cout << "Starting event loop... Press Ctrl+C to stop." << std::endl;
    g_platformManager->startEventLoop(); // This blocks until stopEventLoop() is called by the signal handler

    // --- Cleanup ---
    std::cout << "Event loop finished. Cleaning up..." << std::endl;
    // g_platformManager unique_ptr goes out of scope here, automatically calling the
    // WindowsPlatformManager destructor, which should clean up hooks, the helper window,
    // and any remaining owned Window/Monitor objects.

    std::cout << "Maat Test Application finished." << std::endl;
    return 0;
}
