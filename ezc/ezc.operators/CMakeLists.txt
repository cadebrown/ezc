
add_library(ezc.operators SHARED ezcoperators.c ${EZC_MODULE_SOURCES})

target_link_libraries(ezc.operators m)

install (TARGETS ezc.operators DESTINATION ${EZC_MODULE_DIR})

