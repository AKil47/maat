#ifndef MAAT_CORE_MAAT_MEDIATOR_H
#define MAAT_CORE_MAAT_MEDIATOR_H

#include <vector>
#include <utility>
#include <maat_platform/platform_types.h>

namespace maat {
namespace platform {
class PlatformManager;
class Window;
} // namespace platform

namespace core {
class CoreManager;

class MaatMediator {
public:
    MaatMediator();

    // Component registration
    void registerPlatformManager(maat::platform::PlatformManager& platformManager);
    void registerCoreManager(CoreManager& coreManager);
    // Placeholders for future components
    // void registerInputHandler(InputHandler& inputHandler);
    // void registerConfiguration(Configuration& config);

    // Notifications from PlatformManager
    void notifyOsWindowCreated(maat::platform::Window* window);
    void notifyOsWindowDestroyed(maat::platform::WindowId windowId);
    void notifyOsWindowMonitorChanged(maat::platform::WindowId windowId,
                                      maat::platform::MonitorId monitorId);
    void notifyOsMonitorLayoutChanged();

    // Requests from CoreManager
    void requestApplyLayout(
        const std::vector<std::pair<maat::platform::WindowId, maat::platform::Rect>>& layoutUpdates);

    // Lifecycle control (called by main)
    void initialize();
    void run();
    void shutdown();

private:
    maat::platform::PlatformManager* m_platformManager = nullptr;
    CoreManager* m_coreManager = nullptr;
    // Future component pointers
    // InputHandler* m_inputHandler = nullptr;
    // Configuration* m_configuration = nullptr;
};

} // namespace core
} // namespace maat

#endif // MAAT_CORE_MAAT_MEDIATOR_H
