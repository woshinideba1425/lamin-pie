#pragma once

#include <memory.h>
#include <cstdlib>
#include "lvgl.h"

namespace laminpie::gui {

class LvObject {
public:
    explicit LvObject(lv_obj_t *p) : _native_handle(p) {}
    ~LvObject() {}
    bool setStyleAttribute(const StyleSize &size);

private:
    lv_obj_t *_native_handle;
};

}

