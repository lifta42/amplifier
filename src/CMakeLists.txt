set(AUTOMATA_MODULES dfa)

function(generate_test MOD)
  set(TEST_TARGET ${MOD}_test)
  add_executable(${TEST_TARGET} ${MOD}_test.c)
  target_link_libraries(${TEST_TARGET} automata)
  add_test(test_${MOD} ${TEST_TARGET})
endfunction()

set(AUTOMATA_SOURCES)
foreach(MOD ${AUTOMATA_MODULES})
  list(APPEND AUTOMATA_SOURCES ${MOD}.c)
  generate_test(${MOD})
endforeach()

add_library(automata ${AUTOMATA_SOURCES})
