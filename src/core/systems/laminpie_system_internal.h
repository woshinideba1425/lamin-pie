#pragma once 

#include "../../../lamin_conf.h"
#include "../../laminpie_internal.h"

#if !LAMINPIE_ENABLE_SYSTEMS
#   error "LAMINPIE_ENABLE_SYSTEMS is not enabled, enable it in the menuconfig or lamin_conf.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// 模块日志开关配置 ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 定义主模块日志开关
#if !defined(LAMINPIE_CORE_ENABLE_DEBUG_LOG)
#   if defined(CONFIG_LAMINPIE_CORE_ENABLE_DEBUG_LOG)
#       define LAMINPIE_CORE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_ENABLE_DEBUG_LOG
#   else
#       define LAMINPIE_CORE_ENABLE_DEBUG_LOG  (0)
#   endif
#endif

// 子模块日志开关定义
#if LAMINPIE_CORE_ENABLE_DEBUG_LOG
    // CORE.APP 子模块
    #if !defined(LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    // CORE.DISPLAY 子模块
    #if !defined(LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    // CORE.EVENT 子模块
    #if !defined(LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    // LAMINPIE 特殊模块
    #if !defined(LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    #if !defined(LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif
#else
    // 当主模块日志禁用时，所有子模块日志也禁用
    #define LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG       (0)
    #define LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG   (0)
    #define LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG     (0)
    #define LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG (0)
    #define LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG (0)
#endif

// 便捷的模块日志检查宏
#define LAMINPIE_LOG_ENABLED_CORE_APP       (LAMINPIE_CORE_ENABLE_DEBUG_LOG && LAMINPIE_CORE_APP_ENABLE_DEBUG_LOG)
#define LAMINPIE_LOG_ENABLED_CORE_DISPLAY   (LAMINPIE_CORE_ENABLE_DEBUG_LOG && LAMINPIE_CORE_DISPLAY_ENABLE_DEBUG_LOG)
#define LAMINPIE_LOG_ENABLED_CORE_EVENT     (LAMINPIE_CORE_ENABLE_DEBUG_LOG && LAMINPIE_CORE_EVENT_ENABLE_DEBUG_LOG)
#define LAMINPIE_LOG_ENABLED_CORE_MANAGER    (LAMINPIE_CORE_ENABLE_DEBUG_LOG && LAMINPIE_CORE_MANAGER_ENABLE_DEBUG_LOG)
#define LAMINPIE_LOG_ENABLED_CORE_CORE       (LAMINPIE_CORE_ENABLE_DEBUG_LOG && LAMINPIE_CORE_CORE_ENABLE_DEBUG_LOG)