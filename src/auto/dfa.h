/**
 * \file dfa.h
 * \brief A simple implementation of determine finite automata.
 * \author liftA42
 * \date Dec 10, 2017
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

#include "../naive/lang.h"

typedef int DFAState;
typedef int DFASymbol;

/** The inner representation of a DFA. */
typedef struct _DFAuto *DFAuto;  // all-capital seems weird to me

/** Create a new DFA with no state.

Invoking `dfa_work` directly on its result will cause error.

\return An empty DFA. */
DFAuto dfa_create(void);

/** Create a new state node in a DFA.

\param dfa the extended DFA
\return The state that created for `dfa`. */
DFAState dfa_extend(DFAuto dfa);

/** Draw an arrow between two states of a DFA.

After this invoking, `dfa` will jump from `from` state to `to` state when `with`
symbol is received.

Connecting states that not exists will cause error.

\param dfa the modified DFA
\param from the source state of jumping
\param with the symbol that causes jumping
\param to the destined state of jumping */
void dfa_connect(DFAuto dfa, const DFAState from, const DFASymbol with,
                 const DFAState to);

/** Similiar to `dfa_connect`, only the jumping will occur if none of any
existing symbol matches the received one.

\param dfa the modified DFA
\param from the source state of jumping
\param to the destined state of jumping */
void dfa_connect_fallback(DFAuto dfa, const DFAState from, const DFAState to);

/** Set a state as acceptable state of a DFA.

Passing a non-existent state will cause error.

\param dfa the modified DFA
\param state the acceptable state */
void dfa_accept(DFAuto dfa, const DFAState state);

/** Free a DFA.

\param dfa the freed DFA */
void dfa_destroy(DFAuto dfa);


/** The inner representation of an instance of a DFA.

Multiple instances of a DFA may be created, each of them executes separately,
and leave the original `DFAuto` unchanged. */
typedef struct _DFAInst *DFAInst;

/** Create a `DFAInstance` with a `DFA` and starting state.

Different instances of the same DFA may start executing from vary states.

\param dfa the frozen DFA
\param start the start state of `dfa`
\return The created `DFAInstance` object. */
DFAInst dfa_freeze(const DFAuto dfa, const DFAState start);

/** Send a symbol into a DFA instance and let it transition.

An error will be raised if there's no outgoing arrow matching passed symbol.

\param instance the modified DFA instance
\param symbol the sent symbol
\return The state that DFA instance reaches after receiving `symbol`. */
DFAState dfa_send(DFAInst instance, const DFASymbol symbol);

/** Check whether a DFA instance is on an acceptable state.

\param instance the checked DFA instance
\return The result is `ture` if the passed DFA instance is on an acceptable
state. */
bool dfa_acceptable(const DFAInst instance);

/** Free a DFA instance.

\param instance the freed DFA instance */
void dfa_destory_inst(DFAInst instance);

#endif
