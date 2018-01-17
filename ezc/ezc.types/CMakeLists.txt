

add_library(ezc.types SHARED ezctypes.c ${EZC_MODULE_SOURCES})


if(GMP_FOUND)
  target_link_libraries(ezc.types ${GMP_LIBRARIES})
endif()

if(MPFR_FOUND)
    target_link_libraries(ezc.types ${MPFR_LIBRARIES})
endif()



install (TARGETS ezc.types DESTINATION ${EZC_MODULE_DIR})
