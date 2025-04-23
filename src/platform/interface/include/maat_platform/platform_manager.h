#ifndef MAAT_PLATFORM_PLATFORM_MANAGER_H_
#define MAAT_PLATFORM_PLATFORM_MANAGER_H_

#include <vector>
#include <functional>
#include <utility>

#include "platform_types.h"

namespace maat {
namespace platform {
class Window;
class Monitor;
} 
} 


namespace maat::platform {

class PlatformManager {
public:
    virtual ~PlatformManager() = default;

    /**
     * @brief Applies geometry updates to multiple windows simultaneously.
     * @param updates A vector of pairs, where each pair contains the WindowId
     *                and the desired new Rect (position and size) for that window.
     * @details The implementation should attempt to use OS-specific batching
     *          mechanisms for efficiency and visual consistency.
     */
    virtual void applyWindowGeometries(const std::vector<std::pair<WindowId, Rect> >& updates) = 0;


    /**
     * @brief Enumerates all currently active monitors.
     * @return A vector of non-owning pointers to Monitor objects.
     *         The PlatformManager implementation retains ownership.
     */
    virtual std::vector<Monitor*> enumerateMonitors() = 0;

    /**
     * @brief Enumerates all existing top-level windows that might be manageable.
     * @return A vector of non-owning pointers to Window objects.
     *         The PlatformManager implementation retains ownership.
     * @note Core logic should call window->isManageable() to filter further.
     */
    virtual std::vector<Window*> enumerateInitialWindows() = 0;


    /**
     * @brief Sets the callback function to be invoked when a new window is detected.
     * @param cb Function taking a non-owning Window pointer (owned by PlatformManager).
     */
    virtual void setWindowCreatedCallback(std::function<void(Window*)> cb) = 0;

    /**
     * @brief Sets the callback function to be invoked when a tracked window is destroyed.
     * @param cb Function taking the WindowId of the destroyed window.
     */
    virtual void setWindowDestroyedCallback(std::function<void(WindowId)> cb) = 0;

    /**
     * @brief Sets the callback function for when a tracked window changes monitor significantly.
     * @param cb Function taking the WindowId and the likely new MonitorId.
     */
    virtual void setWindowMonitorChangedCallback(std::function<void(WindowId, MonitorId)> cb) = 0;

    /**
     * @brief Sets the callback function for when monitor layout/configuration changes.
     * @param cb Function to be invoked. Core should re-enumerate monitors upon receiving this.
     */
    virtual void setMonitorLayoutChangedCallback(std::function<void()> cb) = 0;


    /**
     * @brief Informs the PlatformManager that the Core is no longer tracking this WindowId.
     * @param id The ID of the window (typically one that was just destroyed).
     * @details Allows the PlatformManager implementation to potentially release
     *          internal resources associated with the window object.
     */
    virtual void releaseWindowTracking(WindowId id) = 0;


    /**
     * @brief Starts the platform-specific event loop.
     * @details This function typically blocks until stopEventLoop() is called
     *          or a shutdown event occurs. It listens for OS events and triggers
     *          the registered callbacks.
     */
    virtual void startEventLoop() = 0;

    /**
     * @brief Signals the running event loop to terminate gracefully.
     * @details May need to be called from a different thread or signal handler
     *          depending on the event loop implementation.
     */
    virtual void stopEventLoop() = 0;

};

} 

#endif 
