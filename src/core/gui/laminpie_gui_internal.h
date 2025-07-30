#pragma once

#include "laminpie_conf.h"
#include "laminpie_log.hpp"

#define LAMINPIE_LOG_TAG "LAMINPIE:GUI"

#if LAMINPIE_GUI_ENABLE_DEBUG_LOG
#   define LAMINPIE_GUI_LOG_TRACE(fmt, ...) LP_MOD_LOG_TRACE(LAMINPIE_LOG_TAG, LAMINPIE_GUI_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_GUI_LOG_DEBUG(fmt, ...) LP_MOD_LOG_DEBUG(LAMINPIE_LOG_TAG, LAMINPIE_GUI_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_GUI_LOG_INFO(fmt, ...) LP_MOD_LOG_INFO(LAMINPIE_LOG_TAG, LAMINPIE_GUI_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_GUI_LOG_WARN(fmt, ...) LP_MOD_LOG_WARN(LAMINPIE_LOG_TAG, LAMINPIE_GUI_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#   define LAMINPIE_GUI_LOG_ERROR(fmt, ...) LP_MOD_LOG_ERROR(LAMINPIE_LOG_TAG, LAMINPIE_GUI_ENABLE_DEBUG_LOG, fmt, ##__VA_ARGS__)
#else
#   define LAMINPIE_GUI_LOG_TRACE(fmt, ...) ((void)0)
#   define LAMINPIE_GUI_LOG_DEBUG(fmt, ...) ((void)0)
#   define LAMINPIE_GUI_LOG_INFO(fmt, ...) ((void)0)
#   define LAMINPIE_GUI_LOG_WARN(fmt, ...) ((void)0)
#   define LAMINPIE_GUI_LOG_ERROR(fmt, ...) ((void)0)
#endif