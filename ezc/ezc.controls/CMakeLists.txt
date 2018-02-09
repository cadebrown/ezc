

add_library(ezc.controls SHARED ezccontrols.c ${EZC_MODULE_SOURCES})

install (TARGETS ezc.controls DESTINATION ${EZC_MODULE_DIR})

