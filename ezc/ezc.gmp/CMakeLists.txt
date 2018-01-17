
add_library(ezc.gmp SHARED ezcgmp.c ${EZC_MODULE_SOURCES})

target_link_libraries(ezc.gmp m mpfr gmp)

install (TARGETS ezc.gmp DESTINATION ${EZC_MODULE_DIR})



