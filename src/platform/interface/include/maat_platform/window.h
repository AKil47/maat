#ifndef MAAT_PLATFORM_WINDOW_H_
#define MAAT_PLATFORM_WINDOW_H_

#include "platform_types.h"

namespace maat::platform {

class Window {
public:
    virtual ~Window() = default;

    virtual WindowId getId() const = 0;
    virtual Rect getGeometry() const = 0;
    virtual bool isManageable() const = 0;
};

}

#endif
