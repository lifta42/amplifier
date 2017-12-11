// Created by liftA42 on Dec 11, 2017.
#include "dfa.h"
#include <assert.h>
#include <stddef.h>


int main()
{
  // The following DFA has three states. It starts from `q0` and expecting
  // a series of `0` and `1` as input symbols. It reaches the acceptable state,
  // aka `q2`, if and only if it gets a `01` pair.
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

  DFAInst inst = dfa_freeze(dfa, q0);
  int serial[] = {1, 1, 0, 0, 0, 1, 1, 0, 1, 0};
  int accept[] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1};  // could use `bool`, but ugly
  for (int i = 0; i < sizeof(serial) / sizeof(int); i++)
  {
    dfa_send(inst, serial[i]);
    assert(dfa_acceptable(inst) == accept[i]);
  }

  dfa_destory_inst(inst);
  dfa_destroy(dfa);
}
