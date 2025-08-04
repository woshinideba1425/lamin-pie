/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>
#include <cstdlib>
#include "lvgl.h"
#include "style/laminpie_gui_style.hpp"
#include "laminpie_lv_object.hpp"

namespace laminpie::gui {

class LvScreen: public LvObject {
public:
    using LvObject::LvObject;

    LvScreen();

    bool load();
};

using LvScreenUniquePtr = std::unique_ptr<LvScreen>;

} // namespace laminpie::gui
