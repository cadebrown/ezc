

add_library(ezc.functions SHARED ezcfunctions.c ${EZC_MODULE_SOURCES})

target_link_libraries(ezc.functions m)

install (TARGETS ezc.functions DESTINATION ${EZC_MODULE_DIR})

