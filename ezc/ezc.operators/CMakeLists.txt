

add_library(ezc.operators SHARED ezcoperators.c ${EZC_MODULE_SOURCES})

if (GMP_FOUND)
    target_link_libraries(ezc.operators ${GMP_LIBRARIES})
endif()

if (MPFR_FOUND)
    target_link_libraries(ezc.operators ${MPFR_LIBRARIES})
endif()


target_link_libraries(ezc.operators m)

install (TARGETS ezc.operators DESTINATION ${EZC_MODULE_DIR})

