#pragma once

/**
 * @brief This file contains utility functions for internal use only and should not be included by other files
 */

#include "laminpie_gui_internal.h"

#define LAMINPIE_LOG_TAG "LAMINPIE:GUIStyle"

#if LAMINPIE_STYLE_ENABLE_DEBUG_LOG
#   define LAMINPIE_STYLE_LOG_TRACE(fmt, ...) LP_MOD_LOG_TRACE(LAMINPIE_LOG_TAG, LAMINPIE_STYLE_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_STYLE_LOG_DEBUG(fmt, ...) LP_MOD_LOG_DEBUG(LAMINPIE_LOG_TAG, LAMINPIE_STYLE_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_STYLE_LOG_INFO(fmt, ...) LP_MOD_LOG_INFO(LAMINPIE_LOG_TAG, LAMINPIE_STYLE_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_STYLE_LOG_WARN(fmt, ...) LP_MOD_LOG_WARN(LAMINPIE_LOG_TAG, LAMINPIE_STYLE_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_STYLE_LOG_ERROR(fmt, ...) LP_MOD_LOG_ERROR(LAMINPIE_LOG_TAG, LAMINPIE_STYLE_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#else
#   define LAMINPIE_STYLE_LOG_TRACE(fmt, ...) ((void)0)
#   define LAMINPIE_STYLE_LOG_DEBUG(fmt, ...) ((void)0)
#   define LAMINPIE_STYLE_LOG_INFO(fmt, ...) ((void)0)
#   define LAMINPIE_STYLE_LOG_WARN(fmt, ...) ((void)0)
#   define LAMINPIE_STYLE_LOG_ERROR(fmt, ...) ((void)0)
#endif

namespace laminpie::gui{
    inline void StyleErrorLog(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        LAMINPIE_STYLE_LOG_ERROR(fmt, args);
        va_end(args);
    }

#define CheckValueAndReturn(value, min, max, returnValue, format, ...) \
    utils::CheckValueAndReturn(value, min, max, returnValue, StyleErrorLog, format, ##__VA_ARGS__)

#define CheckNullAndReturn(value, returnValue, format, ...) \
    utils::CheckNullAndReturn(value, returnValue, StyleErrorLog, format, ##__VA_ARGS__)
}
