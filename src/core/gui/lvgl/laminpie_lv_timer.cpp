/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "laminpie_gui_internal.h"
#if !LAMINPIE_LVGL_OBJECT_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_UTILS_DISABLE_DEBUG_LOG
#endif
#include "private/laminpie_lv_utils.hpp"
#include "laminpie_lv_helper.hpp"
#include "laminpie_lv_timer.hpp"

namespace laminpie::gui {

LvTimer::LvTimer(TimerCallback callback, uint32_t period, void *user_data):
    _callback(callback),
    _user_data{this, user_data}
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();
    LAMINPIE_LVGL_LOG_DEBUG("Param: callback(0x%p), period(%u), user_data(0x%p)", callback, period, user_data);

    CheckNullExit(callback, "Invalid callback");
    _native_handle = lv_timer_create([](lv_timer_t *t) {
        LAMINPIE_LVGL_LOG_TRACE_ENTER();

        LvTimer *timer = (LvTimer *)t->user_data;
        CheckNullExit(timer, "Invalid timer");

        timer->_callback(timer->_user_data.user_data);

        LAMINPIE_LVGL_LOG_TRACE_EXIT();
    }, period, this);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

LvTimer::~LvTimer()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    if (isValid()) {
        lv_timer_delete(_native_handle);
    }

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();
}

bool LvTimer::pause()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid timer");

    lv_timer_pause(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool LvTimer::resume()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid timer");

    lv_timer_resume(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool LvTimer::restart()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid timer");

    lv_timer_reset(_native_handle);
    lv_timer_resume(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool LvTimer::reset()
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid timer");

    lv_timer_reset(_native_handle);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}

bool LvTimer::setInterval(uint32_t interval_ms)
{
    LAMINPIE_LVGL_LOG_TRACE_ENTER_WITH_THIS();

    CheckFalseReturn(isValid(), false, "Invalid timer");
    LAMINPIE_LVGL_LOG_DEBUG("Param: interval_ms(%u)", interval_ms);

    lv_timer_set_period(_native_handle, interval_ms);

    LAMINPIE_LVGL_LOG_TRACE_EXIT_WITH_THIS();

    return true;
}
} // namespace laminpie::gui
