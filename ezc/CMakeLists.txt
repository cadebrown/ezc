
add_subdirectory (systypes) 
add_subdirectory (dummy) 

add_executable(ezc ezc.c module_loader.c estack.c findexecdir.c findexecdir.h estack.h ezctypes.h ezc.h module_loader.h)

target_link_libraries(ezc ${CMAKE_DL_LIBS})
target_link_libraries(ezc systypes ${CMAKE_DL_LIBS})
target_link_libraries(ezc dummy ${CMAKE_DL_LIBS})

install (TARGETS ezc DESTINATION bin)
