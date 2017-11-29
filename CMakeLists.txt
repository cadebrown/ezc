cmake_minimum_required (VERSION 2.6)

project (ezc)
set (EZC_VERSION_MAJOR 0)
set (EZC_VERSION_MINOR 0)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "-std=gnu++11 ${CMAKE_C_FLAGS}")
  endif ()
else ()
  set (CMAKE_C_STANDARD 11)
endif ()

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


