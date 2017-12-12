function(process_units MODULE_NAME UNITS)
  set(MODULE_SOURCES)
  foreach(UNIT ${UNITS})
    list(APPEND MODULE_SOURCES ${UNIT}.c)

    set(TEST_TARGET ${UNIT}_test)
    add_executable(${TEST_TARGET} ${UNIT}_test.c)
    target_link_libraries(${TEST_TARGET} ${MODULE_NAME})
    add_test(test_${MODULE_NAME}_${UNIT} ${TEST_TARGET})
  endforeach()

  add_library(${MODULE_NAME} ${MODULE_SOURCES})
endfunction()

add_subdirectory(auto)
add_subdirectory(naive)
