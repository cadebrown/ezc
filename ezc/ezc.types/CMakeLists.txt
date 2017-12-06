
add_library(ezc.types SHARED ezctypes.c ${EZC_MODULE_SOURCES})

install (TARGETS ezc.types DESTINATION ${EZC_MODULE_DIR})
