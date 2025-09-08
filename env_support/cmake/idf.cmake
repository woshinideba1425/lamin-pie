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

# Register component
idf_component_register(SRCS ${SRCS_C} ${SRCS_CPP}
                #   SRC_DIRS ${LAMINPIE_SRC_DIRS}
                  INCLUDE_DIRS ${LAMINPIE_INC_DIRS}
                  REQUIRES ${LAMINPIE_REQUIRES}
                  PRIV_REQUIRES ${LAMINPIE_PRIV_REQUIRES}
)

# 主测试开关 - 控制是否编译测试程序
option(BUILD_TESTS "Build test programs" ON)

# 如果测试被启用，转到测试目录进行编译
if(BUILD_TESTS)
    message(STATUS "Building LaminPie tests...")
    
    # 添加测试子目录
    add_subdirectory(${LAMINPIE_ROOT_DIR}/test)
    
    # 打印测试配置信息
    message(STATUS "=== LaminPie Test Configuration ===")
    message(STATUS "Tests enabled: ${BUILD_TESTS}")
    message(STATUS "Test directory: ${LAMINPIE_ROOT_DIR}/test")
    message(STATUS "===================================")
else()
    message(STATUS "Tests are disabled by BUILD_TESTS=OFF")
endif()