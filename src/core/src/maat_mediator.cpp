#include "maat_core/maat_mediator.h"
#include <iostream>

#include "maat_platform/platform_manager.h"
#include "maat_core/core_manager.h"

namespace maat {
namespace core {

MaatMediator::MaatMediator() {
    // Default constructor
}

// Component registration
void MaatMediator::registerPlatformManager(maat::platform::PlatformManager& platformManager) {
    m_platformManager = &platformManager;
    std::cout << "[MaatMediator] PlatformManager registered\n";
}

void MaatMediator::registerCoreManager(CoreManager& coreManager) {
    m_coreManager = &coreManager;
    std::cout << "[MaatMediator] CoreManager registered\n";
}

// Notifications from PlatformManager
void MaatMediator::notifyOsWindowCreated(maat::platform::Window* window) {
    std::cout << "[MaatMediator] OS window created: " << window << "\n";
    // TODO: route to CoreManager or other components
}

void MaatMediator::notifyOsWindowDestroyed(maat::platform::WindowId windowId) {
    std::cout << "[MaatMediator] OS window destroyed: " << windowId << "\n";
    // TODO: handle cleanup
}

void MaatMediator::notifyOsWindowMonitorChanged(maat::platform::WindowId windowId,
                                                maat::platform::MonitorId monitorId) {
    std::cout << "[MaatMediator] Window " << windowId
              << " moved to monitor " << monitorId << "\n";
    // TODO: update layout or notify CoreManager
}

void MaatMediator::notifyOsMonitorLayoutChanged() {
    std::cout << "[MaatMediator] OS monitor layout changed\n";
    // TODO: query PlatformManager for new layout
}

// Requests from CoreManager
void MaatMediator::requestApplyLayout(
    const std::vector<std::pair<maat::platform::WindowId, maat::platform::Rect>>& layoutUpdates) {
    std::cout << "[MaatMediator] Applying layout updates (" << layoutUpdates.size() << " entries)\n";
    // TODO: forward layout commands to PlatformManager
}

// Lifecycle control (called by main)
void MaatMediator::initialize() {
    std::cout << "[MaatMediator] Initialization started\n";
    // TODO: perform any setup before event loop
    if (m_platformManager) {
        auto initialWindows = m_platformManager->enumerateInitialWindows();
        for (auto* window : initialWindows) {
            notifyOsWindowCreated(window);
        }
    }
}

void MaatMediator::run() {
    std::cout << "[MaatMediator] Running main loop\n";
    if (m_platformManager) {
        m_platformManager->startEventLoop();
    } else {
        std::cerr << "[MaatMediator] No PlatformManager to start event loop\n";
    }
}

void MaatMediator::shutdown() {
    std::cout << "[MaatMediator] Shutdown initiated\n";
    if (m_platformManager) {
        m_platformManager->stopEventLoop();
    } else {
        std::cerr << "[MaatMediator] No PlatformManager to stop event loop\n";
    }
}

} // namespace core
} // namespace maat
