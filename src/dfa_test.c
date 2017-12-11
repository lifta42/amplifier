// Created by liftA42 on Dec 11, 2017.
#include "dfa.h"
#include <assert.h>
#include <stddef.h>


int main()
{
  DFAuto dfa = dfa_create();

  DFAState q0 = dfa_extend(dfa);
  DFAState q1 = dfa_extend(dfa);
  DFAState q2 = dfa_extend(dfa);

  dfa_connect(dfa, q0, 1, q0);
  dfa_connect(dfa, q0, 0, q1);
  dfa_connect(dfa, q1, 0, q1);
  dfa_connect(dfa, q1, 1, q2);
  dfa_connect(dfa, q2, 0, q2);
  dfa_connect(dfa, q2, 1, q2);

  dfa_accept(dfa, q2);

  dfa_destroy(dfa);
}
