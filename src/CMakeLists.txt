add_library(automata dfa.c)

add_executable(automata_test dfa_test.c)
target_link_libraries(automata_test automata)

add_test(test_dfa automata_test)
