
add_library(ezc.io SHARED ezcio.c ${EZC_MODULE_SOURCES})

install (TARGETS ezc.io DESTINATION ${EZC_MODULE_DIR})
