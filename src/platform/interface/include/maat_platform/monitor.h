#ifndef MAAT_PLATFORM_MONITOR_H_
#define MAAT_PLATFORM_MONITOR_H_

#include "platform_types.h"

namespace maat { namespace platform {

class Monitor {
public:
    virtual ~Monitor() = default;

    virtual MonitorId getId() const = 0;
    virtual Rect getWorkArea() const = 0;
};

} }

#endif
