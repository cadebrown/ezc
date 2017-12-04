cmake_minimum_required (VERSION 2.6)

project (ezc)
set (EZC_VERSION_MAJOR 0)
set (EZC_VERSION_MINOR 1)


set (CMAKE_C_FLAGS "-O2 -g -Wall -pedantic -std=c99 ${CMAKE_C_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

set(EZC_MODULE_DIR "lib/ezc" CACHE STRING "Where to install ezc modules that are build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")


find_package(Readline)
find_package(GMP)
find_package(MPFR)


if(READLINE_FOUND)
  include_directories(${Readline_INCLUDE_DIR})
  add_definitions(-DHAVE_READLINE)
endif()


if(GMP_FOUND)
  include_directories(${GMP_INCLUDE_DIR})
  add_definitions(-DHAVE_GMP)
endif()


if(MPFR_FOUND)
  include_directories(${MPFR_INCLUDES})
  add_definitions(-DHAVE_MPFR)
endif()




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

add_subdirectory (ezc) 
add_subdirectory (examples)

install (FILES "${PROJECT_BINARY_DIR}/ezcconfig.h" DESTINATION include)


