/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "laminpie_gui_internal.h"
#if !LAMINPIE_LVGL_CANVAS_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_canvas.hpp"

namespace laminpie::gui {

LvCanvas::LvCanvas(const LvObject *parent):
    LvObject((parent != nullptr) ? (lv_canvas_create(parent->getNativeHandle())) : nullptr)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: parent(0x%p)", parent);

    CheckFalseExit(isValid(), "Failed to create canvas");

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

bool LvCanvas::setBuffer(void *buffer, int width, int height)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: buffer(0x%p), width(%d), height(%d)", buffer, width, height);

    CheckFalseReturn(isValid(), false, "Invalid object");
    CheckNullAndReturn(buffer, false, "Invalid buffer");

    lv_canvas_set_buffer(getNativeHandle(), buffer, width, height, LV_COLOR_FORMAT_NATIVE);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

} // namespace laminpie::gui
