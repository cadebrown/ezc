
set(EZC_MODULE_SOURCES "${CMAKE_SOURCE_DIR}/ezc/ezcmodule.h")

add_subdirectory (ezc.types) 
add_subdirectory (ezc.functions)
add_subdirectory (ezc.io)

add_library(ezc ezc.c module_loader.c log.c exec.c exec.h estack.c dict.c routines.c whereami.c routines.h whereami.h estack.h dict.h ezctypes.h log.h ezc.h module_loader.h)

install (TARGETS ezc DESTINATION lib)


add_executable(ec ec.c ec.h)

target_link_libraries(ec ezc ${CMAKE_DL_LIBS})


install (TARGETS ec DESTINATION bin)


