/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
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

class LvContainer: public LvObject {
public:
    LvContainer(const LvObject *parent);
};

using LvContainerUniquePtr = std::unique_ptr<LvContainer>;
using LvContainerSharedPtr = std::shared_ptr<LvContainer>;

} // namespace laminpie::gui
