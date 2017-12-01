

add_library(ezc.functions SHARED ezcfunctions.c ${EZC_MODULE_SOURCES})

install (TARGETS ezc.functions DESTINATION ${EZC_MODULE_DIR})

