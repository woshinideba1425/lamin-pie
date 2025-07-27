# idf CMakeLists.txt

# LAMINATEPIE
file(GLOB_RECURSE LAMINATEPIE_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/app/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/simplekv_nvs/simplekv.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/framework/*.cpp
)
set(LAMINATEPIE_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/
    ${LAMINATEPIE_ROOT_DIR}/src/app/
    ${LAMINATEPIE_ROOT_DIR}/src/simplekv_nvs/
    ${LAMINATEPIE_ROOT_DIR}/src/framework/
)

#SYSTERM_RESOUCE
file(GLOB_RECURSE SYSTERM_RESOUCE_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/*.c

)
set(LAMINATEPIE_SYSTERM_RESOUCE_SRCS_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/system_resouce/
)

#PMSystem
file(GLOB_RECURSE PMSystem_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/PM/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/PM/*.c

)
set(LAMINATEPIE_PM_SRCS_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/PM/
)

# DMODEL
file(GLOB_RECURSE DMODEL_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/*.c
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/interface/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/interface/*.c
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/test/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/test/*.c
)
set(LAMINATEPIE_DMODEL_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/interface/
    ${LAMINATEPIE_ROOT_DIR}/src/dmodel/test/
)

#COMMON
file(GLOB_RECURSE COMMON_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/common/*.cpp
    ${LAMINATEPIE_ROOT_DIR}/src/common/*.c
)
set(LAMINATEPIE_COMMON_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/common/
)
# Service
file(GLOB_RECURSE SERVICE_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/service/*.cpp
)
set(LAMINATEPIE_SERVICE_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/service/
)

# Built-in Apps
file(GLOB_RECURSE APP_BUILT_SRCS
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/*.c
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/*.cpp
)
set(LAMINATEPIE_APP_BUILT_INC_DIRS
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_oxyzen/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_pressure/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/blood_oxyzen/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/weather/
    ${LAMINATEPIE_ROOT_DIR}/src/app_built/lanucher/
)

# Public component requirement
set(LAMINATEPIE_REQUIRES
    lvgl nvs_flash main ui
)

# Private component requirement
set(LAMINATEPIE_PRIV_REQUIRES
)

# Register component
idf_component_register(SRCS ${LAMINATEPIE_SRCS} ${SYSTERM_RESOUCE_SRCS} ${PMSystem_SRCS} ${APP_BUILT_SRCS} ${SERVICE_SRCS} ${DMODEL_SRCS} ${COMMON_SRCS} ${LAMINATEPIE_ROOT_DIR}/src/laminatepie.cpp
                #   SRC_DIRS ${LAMINATEPIE_SRC_DIRS}
                  INCLUDE_DIRS ${LAMINATEPIE_INC_DIRS} ${LAMINATEPIE_PM_SRCS_INC_DIRS} ${LAMINATEPIE_APP_BUILT_INC_DIRS} ${LAMINATEPIE_SYSTERM_RESOUCE_SRCS_INC_DIRS} ${LAMINATEPIE_SERVICE_INC_DIRS} ${LAMINATEPIE_DMODEL_INC_DIRS} ${LAMINATEPIE_COMMON_INC_DIRS}
                  REQUIRES ${LAMINATEPIE_REQUIRES}
                  PRIV_REQUIRES ${LAMINATEPIE_PRIV_REQUIRES}
)
