
add_subdirectory (systypes) 
add_subdirectory (std_func)

add_library(ezc ezc.c module_loader.c estack.c dict.c routines.c whereami.c routines.h whereami.h estack.h dict.h ezctypes.h ezc.h module_loader.h)
install (TARGETS ezc DESTINATION lib/ezc)

add_executable(ec ec.c ec.h)


target_link_libraries(ec ezc ${CMAKE_DL_LIBS})


install (TARGETS ec DESTINATION bin)


