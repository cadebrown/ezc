
set(EZC_MODULE_SOURCES "${CMAKE_SOURCE_DIR}/ezc/ezcmodule.h")

add_subdirectory (ezc.types) 
add_subdirectory (ezc.functions)
add_subdirectory (ezc.io)
add_subdirectory (ezc.operators)

add_library(ezc ezc.c module_loader.c log.c exec.c exec.h estack.c dict.c routines.c whereami.c routines.h whereami.h estack.h dict.h ezctypes.h log.h ezc.h module_loader.h)

install (TARGETS ezc DESTINATION lib)


add_executable(ec ec.c ec.h)

target_link_libraries(ec ezc ${CMAKE_DL_LIBS})

if(READLINE_FOUND)
    target_link_libraries(ec ${Readline_LIBRARY})
endif()


install (FILES ezc.h DESTINATION include)

install (FILES dict.c DESTINATION include/ezc)
install (FILES dict.h DESTINATION include/ezc)
install (FILES estack.c DESTINATION include/ezc)
install (FILES estack.h DESTINATION include/ezc)
install (FILES routines.c DESTINATION include/ezc)
install (FILES routines.h DESTINATION include/ezc)

install (FILES ezclang.h DESTINATION include)
install (FILES ezcmacros.h DESTINATION include)
install (FILES ezcsymboldefs.h DESTINATION include)
install (FILES ezcmodule.h DESTINATION include)

install (TARGETS ec DESTINATION bin)


