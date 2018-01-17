
add_library(ezc.operators SHARED ezcoperators.c ${EZC_MODULE_SOURCES})

target_link_libraries(ezc.operators m)

if(GMP_FOUND)
  target_link_libraries(ezc.operators ${GMP_LIBRARIES})
endif()

if(MPFR_FOUND)
    target_link_libraries(ezc.operators ${MPFR_LIBRARIES})
endif()


install (TARGETS ezc.operators DESTINATION ${EZC_MODULE_DIR})

