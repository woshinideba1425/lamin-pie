# idf CMakeLists.txt

# LAMINPIE
file(GLOB_RECURSE SOURCES ${LAMINPIE_ROOT_DIR}/src/*.c)
set(SRCS_C "")
set(SRCS_CPP "")
set(LAMINPIE_COMPILE_OPTIONS "")
set(LAMINPIE_INC_DIRS ${LAMINPIE_ROOT_DIR})
set(LAMINPIE_SRC_DIRS ${LAMINPIE_ROOT_DIR}/src)
set(CORE_SRC_DIR ${LAMINPIE_ROOT_DIR}/src/core)

# Public component requirement
set(LAMINPIE_REQUIRES
    lvgl nvs_flash main
)

# Private component requirement
set(LAMINPIE_PRIV_REQUIRES
)
#
# COMMON
#
set(COMMON_SRC_DIR ${LAMINPIE_SRC_DIRS}/common)
file(GLOB_RECURSE COMMON_SRCS_C ${COMMON_SRC_DIR}/*.c)
file(GLOB_RECURSE COMMON_SRCS_CPP ${COMMON_SRC_DIR}/*.cpp)
list(APPEND SRCS_C ${COMMON_SRCS_C})
list(APPEND SRCS_CPP ${COMMON_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${COMMON_SRC_DIR})
list(APPEND LAMINPIE_COMPILE_OPTIONS "-DLAMINPIE_CONF_SKIP")

#
# GUI
#
if(CONFIG_LAMINPIE_ENABLE_GUI)
    set(GUI_SRC_DIR ${CORE_SRC_DIR}/gui)
    file(GLOB_RECURSE GUI_SRCS ${GUI_SRC_DIR}/*.c)
    list(APPEND LAMINPIE_SRCS ${GUI_SRCS})
    list(APPEND LAMINPIE_INC_DIRS ${GUI_SRC_DIR})
    # Animation Player
    if(CONFIG_LAMINPIE_GUI_ENABLE_ANIM_PLAYER)
        set(GUI_ANIM_PLAYER_SRC_DIR ${GUI_SRC_DIR}/anim_player)
        file(GLOB_RECURSE GUI_ANIM_PLAYER_SRCS_C ${GUI_ANIM_PLAYER_SRC_DIR}/*.c)
        file(GLOB_RECURSE GUI_ANIM_PLAYER_SRCS_CPP ${GUI_ANIM_PLAYER_SRC_DIR}/*.cpp)
        list(APPEND SRCS_C ${GUI_ANIM_PLAYER_SRCS_C})
        list(APPEND SRCS_CPP ${GUI_ANIM_PLAYER_SRCS_CPP})
    endif()
    # Squareline
    if(CONFIG_LAMINPIE_GUI_ENABLE_SQUARELINE)
        set(GUI_SQUARELINE_SRC_DIR ${GUI_SRC_DIR}/squareline)
        # UI Components
        if(CONFIG_LAMINPIE_SQUARELINE_ENABLE_UI_COMP)
            set(GUI_SQUARELINE_UI_COMP_SRC_DIR ${GUI_SQUARELINE_SRC_DIR}/ui_comp)
            file(GLOB_RECURSE GUI_SQUARELINE_UI_COMP_SRCS_C ${GUI_SQUARELINE_UI_COMP_SRC_DIR}/*.c)
            list(APPEND SRCS_C ${GUI_SQUARELINE_UI_COMP_SRCS_C})
        endif()
        # UI Helpers
        if(CONFIG_LAMINPIE_SQUARELINE_ENABLE_UI_HELPERS)
            set(GUI_SQUARELINE_UI_HELPERS_SRC_DIR ${GUI_SQUARELINE_SRC_DIR}/ui_helpers)
            file(GLOB_RECURSE GUI_SQUARELINE_UI_HELPERS_SRCS_C ${GUI_SQUARELINE_UI_HELPERS_SRC_DIR}/*.c)
            list(APPEND SRCS_C ${GUI_SQUARELINE_UI_HELPERS_SRCS_C})
        endif()
    endif()
    # LVGL
    set(GUI_LVGL_SRC_DIR ${GUI_SRC_DIR}/lvgl)
    file(GLOB_RECURSE GUI_LVGL_SRCS_C ${GUI_LVGL_SRC_DIR}/*.c)
    file(GLOB_RECURSE GUI_LVGL_SRCS_CPP ${GUI_LVGL_SRC_DIR}/*.cpp)
    list(APPEND SRCS_C ${GUI_LVGL_SRCS_C})
    list(APPEND SRCS_CPP ${GUI_LVGL_SRCS_CPP})
    list(APPEND LAMINPIE_COMPILE_OPTIONS "-DLV_LVGL_H_INCLUDE_SIMPLE")
    # Style
    set(GUI_STYLE_SRC_DIR ${GUI_SRC_DIR}/style)
    file(GLOB_RECURSE GUI_STYLE_SRCS_C ${GUI_STYLE_SRC_DIR}/*.c)
    file(GLOB_RECURSE GUI_STYLE_SRCS_CPP ${GUI_STYLE_SRC_DIR}/*.cpp)
    list(APPEND SRCS_C ${GUI_STYLE_SRCS_C})
    list(APPEND SRCS_CPP ${GUI_STYLE_SRCS_CPP})
endif()

#
# DEVICE
#
if(CONFIG_LAMINPIE_ENABLE_DEVICE)
    set(DEVICE_SRC_DIR ${LAMINPIE_SRC_DIRS}/device)
    set(BUS_SRC_DIR ${DEVICE_SRC_DIR}/buses)
    set(DRIVER_SRC_DIR ${DEVICE_SRC_DIR}/drivers)
    set(I2C_SRC_DIR ${DRIVER_SRC_DIR}/i2c)
    file(GLOB_RECURSE DEVICE_SRCS_C ${DEVICE_SRC_DIR}/*.c)
    file(GLOB_RECURSE DEVICE_SRCS_CPP ${DEVICE_SRC_DIR}/*.cpp)
    list(APPEND SRCS_C ${DEVICE_SRCS_C})
    list(APPEND SRCS_CPP ${DEVICE_SRCS_CPP})
    list(APPEND LAMINPIE_INC_DIRS ${DEVICE_SRC_DIR} ${BUS_SRC_DIR} ${DRIVER_SRC_DIR} ${I2C_SRC_DIR})    
endif()

#
# systems
#
if(CONFIG_LAMINPIE_ENABLE_SYSTEMS)
    set(SYSTEMS_SRC_DIR ${CORE_SRC_DIR}/systems)
    set(APP_SRC_DIR ${SYSTEMS_SRC_DIR}/app)
    set(FRAMEWORK_SRC_DIR ${SYSTEMS_SRC_DIR}/framework)
    file(GLOB_RECURSE SYSTEMS_SRCS_C ${SYSTEMS_SRC_DIR}/*.c)
    file(GLOB_RECURSE SYSTEMS_SRCS_CPP ${SYSTEMS_SRC_DIR}/*.cpp)
    list(APPEND SRCS_C ${SYSTEMS_SRCS_C})
    list(APPEND SRCS_CPP ${SYSTEMS_SRCS_CPP})
    list(APPEND LAMINPIE_INC_DIRS ${SYSTEMS_SRC_DIR} ${APP_SRC_DIR} ${FRAMEWORK_SRC_DIR} ${SYSTEM_SRC_DIR} ${UI_SRC_DIR} ${UI_COMP_SRC_DIR} ${UI_HELPERS_SRC_DIR})
endif()

#
# thread porting
#
set(PORTING_SRC_DIR ${LAMINPIE_SRC_DIRS}/porting)

# 根据LAMINPIE_USE_OS宏选择具体的实现文件
if(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_NONE")
    # 无操作系统支持
    set(PORTING_SRCS_C ${PORTING_SRC_DIR}/laminpie_none.c)
    set(PORTING_SRCS_CPP "")
elseif(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_STD_THREAD")
    # 标准C++线程库
    set(PORTING_SRCS_C "")
    set(PORTING_SRCS_CPP ${PORTING_SRC_DIR}/laminpie_std_thread.cpp)
elseif(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_PTHREAD")
    # POSIX线程库
    set(PORTING_SRCS_C "")
    set(PORTING_SRCS_CPP ${PORTING_SRC_DIR}/laminpie_pthread.cpp)
elseif(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_FREERTOS")
    # FreeRTOS实时操作系统
    set(PORTING_SRCS_C ${PORTING_SRC_DIR}/laminpie_freertos.cpp)
    set(PORTING_SRCS_CPP "")
elseif(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_RTTHREAD")
    # RT-Thread实时操作系统
    set(PORTING_SRCS_C ${PORTING_SRC_DIR}/laminpie_rtthread.cpp)
    set(PORTING_SRCS_CPP "")
elseif(CONFIG_LAMINPIE_USE_OS STREQUAL "LAMINPIE_OS_CUSTOM")
    # 自定义操作系统实现
    if(DEFINED LAMINPIE_OS_CUSTOM_SRC)
        set(PORTING_SRCS_C ${LAMINPIE_OS_CUSTOM_SRC})
        set(PORTING_SRCS_CPP "")
    else()
        message(FATAL_ERROR "LAMINPIE_USE_OS is set to LAMINPIE_OS_CUSTOM but LAMINPIE_OS_CUSTOM_SRC is not defined")
    endif()
else()
    # 默认使用标准库线程
    message(STATUS "LAMINPIE_USE_OS not specified, using default std::thread implementation")
    set(PORTING_SRCS_C "")
    set(PORTING_SRCS_CPP ${PORTING_SRC_DIR}/laminpie_std_thread.cpp)
endif()

# 添加选中的源文件
list(APPEND SRCS_C ${PORTING_SRCS_C})
list(APPEND SRCS_CPP ${PORTING_SRCS_CPP})
list(APPEND LAMINPIE_INC_DIRS ${PORTING_SRC_DIR})

# 测试相关编译定义 - 从Kconfig读取配置
if(CONFIG_LAMINPIE_ENABLE_TEST)
    message(STATUS "Building LaminPie tests...")
    
    # 设置测试相关的编译定义 - 从Kconfig读取
    if(CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_LOG_TESTS=1")
        message(STATUS "Log System Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_EVENT_SYSTEM_TEST=1")
        message(STATUS "Event System Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_EVENT_STRESS_TEST=1")
        message(STATUS "Event Stress Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_APP_SCHEDULER_TEST=1")
        message(STATUS "App Scheduler Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_DEVICE_SYSTEM_TEST=1")
        message(STATUS "Device System Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_THREAD_SYSTEM_TEST=1")
        message(STATUS "Thread System Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_PLATFORM_COMPATIBILITY_TEST=1")
        message(STATUS "Platform Compatibility Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_KCONFIG)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_KCONFIG_TEST=1")
        message(STATUS "KConfig Tests: ENABLED")
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE)
        list(APPEND LAMINPIE_COMPILE_OPTIONS "-DENABLE_MEMORY_PERFORMANCE_TEST=1")
        message(STATUS "Memory Performance Tests: ENABLED")
    endif()
    
    # 添加测试源文件
    set(TEST_SRC_DIR ${LAMINPIE_ROOT_DIR}/test)
    set(TEST_UNIT_SRC_DIR ${TEST_SRC_DIR}/unit)
    set(TEST_INTEGRATION_SRC_DIR ${TEST_SRC_DIR}/integration)
    set(TEST_PERFORMANCE_SRC_DIR ${TEST_SRC_DIR}/performance)
    
    # 测试主入口文件
    list(APPEND SRCS_CPP ${TEST_SRC_DIR}/test_main.cpp)
    list(APPEND LAMINPIE_INC_DIRS ${TEST_SRC_DIR}/include)
    
    # 单元测试文件 - 使用Kconfig配置
    if(CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM)
        list(APPEND SRCS_CPP ${TEST_UNIT_SRC_DIR}/test_log_system.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY)
        list(APPEND SRCS_CPP ${TEST_UNIT_SRC_DIR}/test_platform_compatibility.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_KCONFIG)
        list(APPEND SRCS_CPP ${TEST_UNIT_SRC_DIR}/test_kconfig.cpp)
    endif()
    
    # 集成测试文件 - 使用Kconfig配置
    if(CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM)
        list(APPEND SRCS_CPP ${TEST_INTEGRATION_SRC_DIR}/test_event_system.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS)
        list(APPEND SRCS_CPP ${TEST_INTEGRATION_SRC_DIR}/test_event_stress.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER)
        list(APPEND SRCS_CPP ${TEST_INTEGRATION_SRC_DIR}/test_app_scheduler.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM)
        list(APPEND SRCS_CPP ${TEST_INTEGRATION_SRC_DIR}/test_device_system.cpp)
    endif()
    
    if(CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM)
        list(APPEND SRCS_CPP ${TEST_INTEGRATION_SRC_DIR}/test_thread_system.cpp)
    endif()
    
    # 性能测试文件 - 使用Kconfig配置
    if(CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE)
        list(APPEND SRCS_CPP ${TEST_PERFORMANCE_SRC_DIR}/test_memory_performance.cpp)
    endif()
    
    # 打印测试配置信息
    message(STATUS "=== LaminPie Test Configuration ===")
    message(STATUS "Tests enabled: ${CONFIG_LAMINPIE_ENABLE_TEST}")
    message(STATUS "Test directory: ${TEST_SRC_DIR}")
    message(STATUS "Log tests: ${CONFIG_LAMINPIE_TEST_ENABLE_LOG_SYSTEM}")
    message(STATUS "Event system tests: ${CONFIG_LAMINPIE_TEST_ENABLE_EVENT_SYSTEM}")
    message(STATUS "Event stress tests: ${CONFIG_LAMINPIE_TEST_ENABLE_EVENT_STRESS}")
    message(STATUS "App scheduler tests: ${CONFIG_LAMINPIE_TEST_ENABLE_APP_SCHEDULER}")
    message(STATUS "Device system tests: ${CONFIG_LAMINPIE_TEST_ENABLE_DEVICE_SYSTEM}")
    message(STATUS "Thread system tests: ${CONFIG_LAMINPIE_TEST_ENABLE_THREAD_SYSTEM}")
    message(STATUS "Platform compatibility tests: ${CONFIG_LAMINPIE_TEST_ENABLE_PLATFORM_COMPATIBILITY}")
    message(STATUS "KConfig tests: ${CONFIG_LAMINPIE_TEST_ENABLE_KCONFIG}")
    message(STATUS "Memory performance tests: ${CONFIG_LAMINPIE_TEST_ENABLE_MEMORY_PERFORMANCE}")
    message(STATUS "===================================")
else()
    message(STATUS "Tests are disabled by CONFIG_LAMINPIE_ENABLE_TEST=n")
endif()

# Register component
idf_component_register(SRCS ${SRCS_C} ${SRCS_CPP}
                #   SRC_DIRS ${LAMINPIE_SRC_DIRS}
                  INCLUDE_DIRS ${LAMINPIE_INC_DIRS}
                  REQUIRES ${LAMINPIE_REQUIRES}
                  PRIV_REQUIRES ${LAMINPIE_PRIV_REQUIRES}
)

# Set compile options after component registration
if(LAMINPIE_COMPILE_OPTIONS)
    target_compile_options(${COMPONENT_LIB} PRIVATE ${LAMINPIE_COMPILE_OPTIONS})
    message(STATUS "LaminPie compile options: ${LAMINPIE_COMPILE_OPTIONS}")
endif()