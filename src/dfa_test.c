// Created by liftA42 on Dec 11, 2017.
#include "dfa.h"
#include <assert.h>
#include <stddef.h>

int main() {
  DFAuto dfa = dfa_create();
  assert(dfa != NULL);
  dfa_destroy(dfa);
}
