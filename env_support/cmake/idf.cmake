# idf CMakeLists.txt

# LAMINPIE
file(GLOB_RECURSE SOURCES ${LAMINPIE_ROOT_DIR}/src/*.c)
set(LAMINPIE_INC_DIRS
    ${LAMINPIE_ROOT_DIR}/
    ${LAMINPIE_ROOT_DIR}/src/
    ${LAMINPIE_ROOT_DIR}/src/common/
    ${LAMINPIE_ROOT_DIR}/src/core/
    ${LAMINPIE_ROOT_DIR}/src/device/
)

# Public component requirement
set(LAMINPIE_REQUIRES
    lvgl nvs_flash main
)

# Private component requirement
set(LAMINPIE_PRIV_REQUIRES
)

# Register component
idf_component_register(SRCS ${LAMINPIE_SRCS}
                #   SRC_DIRS ${LAMINPIE_SRC_DIRS}
                  INCLUDE_DIRS ${LAMINPIE_INC_DIRS}
                  REQUIRES ${LAMINPIE_REQUIRES}
                  PRIV_REQUIRES ${LAMINPIE_PRIV_REQUIRES}
)
