cmake_minimum_required (VERSION 2.6)

project (ezc)
set (EZC_VERSION_MAJOR 0)
set (EZC_VERSION_MINOR 0)

set (CMAKE_C_FLAGS "-std=c99 ${CMAKE_C_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

configure_file (
  "${PROJECT_SOURCE_DIR}/ezcconfig.h.in"
  "${PROJECT_BINARY_DIR}/ezcconfig.h"
)
 

if (CMAKE_BUILD_TYPE MATCHES RELEASE)
    message("release mode")
else()
    message("dev mode")
    add_definitions(-DEZC_DEV)
endif()

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/ezc")
include_directories("${CMAKE_SOURCE_DIR}/ezc/exec_code")

add_subdirectory (ezc) 

install (FILES "${PROJECT_BINARY_DIR}/ezcconfig.h" DESTINATION include)


