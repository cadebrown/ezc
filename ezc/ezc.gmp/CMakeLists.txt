
add_library(ezc.gmp SHARED ezcgmp.c ${EZC_MODULE_SOURCES})

target_link_libraries(ezc.gmp ${GMP_LIBRARIES})

if(MPFR_FOUND)
    target_link_libraries(ezc.gmp ${MPFR_LIBRARIES})
endif()

target_link_libraries(ezc.gmp m)

install (TARGETS ezc.gmp DESTINATION ${EZC_MODULE_DIR})



