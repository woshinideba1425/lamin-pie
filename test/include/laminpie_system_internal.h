/**
 * @file laminpie_system_internal.h
 * @brief Internal system definitions for testing
 * @author LaminPie Team
 * @date 2024
 */

#ifndef LAMINPIE_SYSTEM_INTERNAL_H
#define LAMINPIE_SYSTEM_INTERNAL_H

#include "lalog.h"

// System module log macros for testing - 直接使用lalog.h的LOGX宏
#define SYSTEM_LOG_INFO(msg) LOGI(msg)
#define SYSTEM_LOG_DEBUG(msg) LOGD(msg)
#define SYSTEM_LOG_ERROR(msg) LOGE(msg)
#define SYSTEM_LOG_WARN(msg) LOGW(msg)

#define SYSTEM_MANAGER_LOG_ERROR(msg) LOGE(msg)
#define SYSTEM_CORE_LOG_WARN(msg) LOGW(msg)

// 模块专用日志宏 - 直接使用lalog.h的LOGX宏
#define LP_LOGI(msg) LOGI(msg)
#define LP_LOGD(msg) LOGD(msg)
#define LP_LOGW(msg) LOGW(msg)
#define LP_LOGE(msg) LOGE(msg)

#endif // LAMINPIE_SYSTEM_INTERNAL_H
