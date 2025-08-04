/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "laminpie_gui_internal.h"
#if !LAMINPIE_LVGL_OBJECT_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_screen.hpp"

namespace laminpie::gui {

LvScreen::LvScreen():
    LvObject(lv_obj_create(nullptr))
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseExit(isValid(), "Create screen failed");

    CheckFalseExit(removeStyle(nullptr), "Remove style failed");
    CheckFalseExit(
        setStyleAttribute(
            StyleFlag::STYLE_FLAG_CLICKABLE | StyleFlag::STYLE_FLAG_SCROLLABLE, false
        ), "Set style attribute failed"
    );

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

bool LvScreen::load()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid screen");

    lv_screen_load(getNativeHandle());

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
    return true;
}

} // namespace laminpie::gui
