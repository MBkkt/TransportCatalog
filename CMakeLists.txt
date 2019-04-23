cmake_minimum_required(VERSION 3.10.2)

# build configurations
option(RELEASE_BUILD "release build option" OFF)
message("RELEASE_BUILD: ${RELEASE_BUILD}")

# generic target
set(CMAKE_SYSTEM_NAME Generic)
project(Transport-Catalog C CXX)

# compiler bins location
get_filename_component(C_COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
set(TARGET_NAME application)

# additional compilation flags
if (RELEASE_BUILD)
    add_compile_options(-Os -g0)
else ()
    add_compile_options(-O0 -g3)
endif (RELEASE_BUILD)

add_compile_options(-std=c++2a)

# source dirs
set(APP_DIR src)
set(LIB_DIR ${APP_DIR}/lib)

# include dirs
include_directories(
        ${APP_DIR}/inc
)

# application source files
add_library(
        app_lib OBJECT
        ${APP_DIR}/main.cpp
)

# additional library source files
add_library(
        sub_lib OBJECT
        ${LIB_DIR}/descriptions.cpp
        ${LIB_DIR}/json.cpp
        ${LIB_DIR}/requests.cpp
        ${LIB_DIR}/sphere.cpp
        ${LIB_DIR}/transport_catalog.cpp
        ${LIB_DIR}/transport_router.cpp
        ${LIB_DIR}/utils.cpp
)


# executable
add_executable(
        ${TARGET_NAME}
        $<TARGET_OBJECTS:app_lib>
        $<TARGET_OBJECTS:sub_lib>
)

# post build commands
add_custom_command(
        TARGET ${TARGET_NAME}
        COMMAND ${C_COMPILER_PATH}/size ${TARGET_NAME}
        COMMENT "--- size of sections ---"
)
