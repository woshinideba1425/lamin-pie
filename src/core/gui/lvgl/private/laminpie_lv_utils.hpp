#pragma once

/**
 * @brief This file contains utility functions for internal use only and should not be included by other files
 */

#include "laminpie_gui_internal.h"

#define LV_LOG_TAG "LAMINPIE:LVGL"

#if LAMINPIE_LVGL_ENABLE_DEBUG_LOG
#   define LAMINPIE_LVGL_LOG_TRACE(fmt, ...) LP_MOD_LOG_TRACE(LV_LOG_TAG, LAMINPIE_LVGL_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_LVGL_LOG_DEBUG(fmt, ...) LP_MOD_LOG_DEBUG(LV_LOG_TAG, LAMINPIE_LVGL_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_LVGL_LOG_INFO(fmt, ...) LP_MOD_LOG_INFO(LV_LOG_TAG, LAMINPIE_LVGL_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_LVGL_LOG_WARN(fmt, ...) LP_MOD_LOG_WARN(LV_LOG_TAG, LAMINPIE_LVGL_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_LVGL_LOG_ERROR(fmt, ...) LP_MOD_LOG_ERROR(LV_LOG_TAG, LAMINPIE_LVGL_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LOG_TRACE_GUARD() LP_LOG_TRACE_GUARD(LV_LOG_TAG)
#   define LOG_TRACE_GUARD_WITH_THIS() LP_LOG_TRACE_GUARD_WITH_THIS(LV_LOG_TAG)
#else
#   define LAMINPIE_LVGL_LOG_TRACE(fmt, ...) ((void)0)
#   define LAMINPIE_LVGL_LOG_DEBUG(fmt, ...) ((void)0)
#   define LAMINPIE_LVGL_LOG_INFO(fmt, ...) ((void)0)
#   define LAMINPIE_LVGL_LOG_WARN(fmt, ...) ((void)0)
#   define LAMINPIE_LVGL_LOG_ERROR(fmt, ...) ((void)0)
#   define LOG_TRACE_GUARD() ((void)0)
#   define LOG_TRACE_GUARD_WITH_THIS() ((void)0)
#endif

namespace laminpie::gui{
    inline void StyleErrorLog(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        LAMINPIE_LVGL_LOG_ERROR(fmt, args);
        va_end(args);
    }

#define CheckValueAndReturn(value, min, max, returnValue, format, ...) \
    utils::CheckValueAndReturn(value, min, max, returnValue, StyleErrorLog, format, ##__VA_ARGS__)

#define CheckNullAndReturn(value, returnValue, format, ...) \
    utils::CheckNullAndReturn(value, returnValue, StyleErrorLog, format, ##__VA_ARGS__)

#define CheckFalseReturn(condition, returnValue, format, ...) \
    utils::CheckFalseReturn(condition, returnValue, StyleErrorLog, format, ##__VA_ARGS__)

#define CheckNullExit(value, format, ...) \
    utils::CheckNullExit(value, StyleErrorLog, format, ##__VA_ARGS__)

#define CheckFalseExit(condition, format, ...) \
    utils::CheckFalseExit(condition, StyleErrorLog, format, ##__VA_ARGS__)
}
