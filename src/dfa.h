/**
 * \file dfa.h
 * \brief A simple implementation of determine finite automata.
 *
 * A DFA is built up with several states and a transition function. The function
 * accepts a state and a symbol as its arguments, and returns another state.
 *
 * After building, a DFA instance is frozen and may work any times you want. For
 * each time it runs, the DFA accepts a series of labels as input and
 * transitions with it. The state of DFA can be traced.
 *
 * Additional callbacks may be set for states. Proper callback will be triggered
 * if the corresponded state is reached.
 */

#ifndef LIFTA42_AUTOMATA_DFA_H
#define LIFTA42_AUTOMATA_DFA_H

typedef int DFAState;
typedef int DFASymbol;

typedef struct _DFAuto *DFAuto;  // all-capital seems weird to me

/// Create a new DFA with no state.
/// Invoking `dfa_work` directly on its result will cause error.
/// \return An empty DFA
DFAuto dfa_create();

/// Create a new state node in a DFA.
/// \param dfa the extended DFA.
/// \return The state that created for `dfa`
DFAState dfa_extend(DFAuto dfa);

/// Draw an arrow between two states of a DFA.
/// After this invoking, `dfa` will jump from `from` state to `to` state when
/// `with` symbol is received.
/// Connecting states that not exists will cause error.
/// \param dfa the modified DFA.
/// \param from the source state of jumping.
/// \param with the symbol that causes jumping.
/// \param to the destined state of jumping.
void dfa_connect(DFAuto dfa, DFAState from, DFASymbol with, DFAState to);

#endif
