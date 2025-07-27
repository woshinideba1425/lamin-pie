# basic CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# LAMINATEPIE
file(GLOB_RECURSE LAMINATEPIE_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/app/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/simplekv_nvs/simplekv_nvs.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/prase/*.cpp
)
set(LAMINATEPIE_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/
    ${LAMINATEPIE_ROOT_DIR}/src/app/
    ${LAMINATEPIE_ROOT_DIR}/src/simplekv_nvs/
    ${LAMINATEPIE_ROOT_DIR}/src/prase/
)

#SYSTERM_RESOUCE
file(GLOB_RECURSE SYSTERM_RESOUCE_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/*.c
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/hal-sys_resouce/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/sys_data/*.cpp

)
set(SYSTERM_RESOUCE_INC
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/
    {LAMINATEPIE_ROOT_DIR}/src/system_resouce/hal-sys_resouce/include
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/sys_data/

)

# Built-in Apps
file(GLOB_RECURSE BUILTIN_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/app/app_built/*.c
    ${LAMINATEPIE_ROOT_DIR}/src/app/app_built/*.cpp
)
set(BUILTIN_INCS
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_oxyzen/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/launcher/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_pressure/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_oxyzen/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/weather/
)

#  Build library
add_library(laminatepie
    ${LAMINATEPIE_ROOT_DIR}/src/laminatepie.cpp
    ${LAMINATEPIE_SRCS}
    ${BUILTIN_SRCS}
    ${SYSTERM_RESOUCE_SRCS}
)

# Include
target_include_directories(laminatepie PUBLIC
    ${LAMINATEPIE_INCS}
    ${BUILTIN_INCS}
    ${SYSTERM_RESOUCE_INC}
)

# Link
target_link_libraries(laminatepie PUBLIC lvgl)