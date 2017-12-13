set(AUTOMATA_MODULES dfa)

process_units(automata ${AUTOMATA_MODULES})
target_link_libraries(automata naive)
