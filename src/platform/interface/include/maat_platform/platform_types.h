#ifndef MAAT_PLATFORM_PLATFORM_TYPES_H_
#define MAAT_PLATFORM_PLATFORM_TYPES_H_

#include <cstdint>

namespace maat { namespace platform {

struct Rect {
    int x;
    int y;
    int width;
    int height;
};

typedef uintptr_t WindowId;
typedef uintptr_t MonitorId;

} }

#endif
