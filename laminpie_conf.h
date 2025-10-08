#pragma once 

// *INDENT-OFF*
#if defined(ESP_PLATFORM)
#include "sdkconfig.h"
#elif defined(PLATFORM_GENERIC)
#include "sdkconfig.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// AI Framework ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_ENABLE_AI_FRAMEWORK)
#   if defined(CONFIG_LAMINPIE_ENABLE_AI_FRAMEWORK)
#       define LAMINPIE_ENABLE_AI_FRAMEWORK  CONFIG_LAMINPIE_ENABLE_AI_FRAMEWORK
#   else
#       define LAMINPIE_ENABLE_AI_FRAMEWORK  (0)
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////// GUI ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_ENABLE_GUI)
#   if defined(CONFIG_LAMINPIE_ENABLE_GUI)
#       define LAMINPIE_ENABLE_GUI  CONFIG_LAMINPIE_ENABLE_GUI
#   else
#       define LAMINPIE_ENABLE_GUI  (0)
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////// Services ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_ENABLE_SERVICES)
#   if defined(CONFIG_LAMINPIE_ENABLE_SERVICES)
#       define LAMINPIE_ENABLE_SERVICES  CONFIG_LAMINPIE_ENABLE_SERVICES
#   else
#       define LAMINPIE_ENABLE_SERVICES  (0)
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////// Systems /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_ENABLE_SYSTEMS)
#   if defined(CONFIG_LAMINPIE_ENABLE_SYSTEMS)
#       define LAMINPIE_ENABLE_SYSTEMS  CONFIG_LAMINPIE_ENABLE_SYSTEMS
#   else
#       define LAMINPIE_ENABLE_SYSTEMS  (0)
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// 操作系统配置 ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* 使用 Kconfig 生成的值 */
#ifndef CONFIG_LAMINPIE_USE_OS_VALUE
#define LAMINPIE_USE_OS CONFIG_LAMINPIE_USE_OS_VALUE
#endif

/* 自定义操作系统包含文件 */
#ifdef LAMINPIE_USE_OS_CUSTOM
#define LAMINPIE_OS_CUSTOM_INCLUDE CONFIG_LAMINPIE_OS_CUSTOM_INCLUDE
#endif

/* 线程配置 */
#define LAMINPIE_THREAD_STACK_SIZE_DEFAULT CONFIG_LAMINPIE_THREAD_STACK_SIZE_DEFAULT
#define LAMINPIE_THREAD_PRIORITY_DEFAULT CONFIG_LAMINPIE_THREAD_PRIORITY_DEFAULT

#if !LAMINPIE_ENABLE_SERVICES
#   error "Services is not enabled, please enable it in the menuconfig"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////     Storage  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(LAMINPIE_SERVICES_ENABLE_STORAGE)
#   if defined(CONFIG_LAMINPIE_SERVICES_ENABLE_STORAGE)
#       define LAMINPIE_SERVICES_ENABLE_STORAGE  CONFIG_LAMINPIE_SERVICES_ENABLE_STORAGE
#   else
#       define LAMINPIE_SERVICES_ENABLE_STORAGE  (0)
#   endif
#endif

#if LAMINPIE_SERVICES_ENABLE_STORAGE
#   if !defined(LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG)
#       if defined(CONFIG_LAMINPIE_STORAGE_ENABLE_DEBUG_LOG)
#           define LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_STORAGE_ENABLE_DEBUG_LOG
#       else
#           define LAMINPIE_SERVICES_STORAGE_ENABLE_DEBUG_LOG  (0)
#       endif
#   endif
#endif


#if !LAMINPIE_ENABLE_SYSTEMS
#   error "LAMINPIE_ENABLE_SYSTEMS is not enabled, enable it in the menuconfig or lamin_conf.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// 模块日志开关配置 ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 定义主模块日志开关
#if !defined(LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG)
#   if defined(CONFIG_LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG)
#       define LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG
#   else
#       define LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG  (0)
#   endif
#endif

// 子模块日志开关定义
#if LAMINPIE_SYSTEM_ENABLE_DEBUG_LOG
    // CORE.APP 子模块
    #if !defined(LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    // CORE.EVENT 子模块
    #if !defined(LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    // LAMINPIE 管理模块
    #if !defined(LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif

    #if !defined(LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG)
    #   if defined(CONFIG_LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG)
    #       define LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG  CONFIG_LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG
    #   else
    #       define LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG  (0)
    #   endif
    #endif
#else
    // 当主模块日志禁用时，所有子模块日志也禁用
    #define LAMINPIE_SYSTEM_APP_ENABLE_DEBUG_LOG       (0)
    #define LAMINPIE_SYSTEM_EVENT_ENABLE_DEBUG_LOG     (0)
    #define LAMINPIE_SYSTEM_MANAGER_ENABLE_DEBUG_LOG (0)
    #define LAMINPIE_SYSTEM_CORE_ENABLE_DEBUG_LOG (0)
#endif