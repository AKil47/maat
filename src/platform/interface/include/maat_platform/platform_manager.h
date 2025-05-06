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


namespace maat { namespace platform {

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

} } 

#endif
