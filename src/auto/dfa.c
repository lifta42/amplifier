// Created by liftA42 on Dec 11, 2017.
#include "../naive/dict.h"
#include "dfa.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct _DFAuto
{
  struct
  {
    Dict transitions;
    DFAState fallback;
    bool acceptable;
  } * states;
  int state_count;
  int state_capacity;
};

// https://stackoverflow.com/questions/35237503/sizeof-anonymous-nested-struct
#define sizeof_field(s, m) (sizeof((((s *)0)->m)))
#define sizeof_field_p(s, pm) (sizeof(*(((s *)0)->pm)))

DFAuto dfa_create(void)
{
  DFAuto dfa = malloc(sizeof(struct _DFAuto));
  assert(dfa != NULL);
  dfa->state_capacity = 16;
  dfa->states =
      malloc(sizeof_field_p(struct _DFAuto, states) * dfa->state_capacity);
  assert(dfa->states != NULL);
  dfa->state_count = 0;
  return dfa;
}

// clang-format off
static dict_gen_hash(hash_symbol, DFASymbol)
static dict_gen_equal(equal_symbol, DFASymbol)

DFAState dfa_extend(DFAuto dfa)
// clang-format on
{
  if (dfa->state_count == dfa->state_capacity)
  {
    dfa->state_capacity *= 2;
    dfa->states = realloc(dfa->states, sizeof_field_p(struct _DFAuto, states)
                                           * dfa->state_capacity);
    assert(dfa->states != NULL);
  }
  // I am regretting for my laziness, that not willing to give the inner struct
  // a name.
  dfa->states[dfa->state_count].transitions =
      dict_create(DFASymbol, DFAState, hash_symbol, equal_symbol);
  dfa->states[dfa->state_count].fallback   = -1;  // no fallback by default
  dfa->states[dfa->state_count].acceptable = false;
  return dfa->state_count++;
}

void dfa_connect(DFAuto dfa, const DFAState from, const DFASymbol with,
                 const DFAState to)
{
  assert(dfa->states[from].transitions != NULL);
  dict_put(dfa->states[from].transitions, &with, &to);
}

void dfa_connect_fallback(DFAuto dfa, const DFAState from, const DFAState to)
{
  assert(dfa->states[from].fallback == -1);  // no override
  dfa->states[from].fallback = to;
}

void dfa_accept(DFAuto dfa, const DFAState state)
{
  dfa->states[state].acceptable = true;
}

void dfa_destroy(DFAuto dfa)
{
  for (int i = 0; i < dfa->state_count; i++)
  {
    dict_destory(dfa->states[i].transitions);
  }
  free(dfa->states);
  free(dfa);
}


struct _DFAInst
{
  const DFAuto dfa;
  DFAState current;
};

DFAInst dfa_freeze(const DFAuto dfa, const DFAState start)
{
  DFAInst inst = malloc(sizeof(struct _DFAInst));
  assert(inst != NULL);
  // Cannot assign directly with `inst->dfa = dfa`, because const member cannot
  // be assigned.
  memcpy(inst, (&(struct _DFAInst){.dfa = dfa, .current = start}),
         sizeof(struct _DFAInst));
  return inst;
}

DFAState dfa_send(DFAInst inst, const DFASymbol symbol)
{
  DFAState dest, *value = dict_get(inst->dfa->states[inst->current].transitions,
                                   &symbol, NULL);
  if (value == NULL)
  {
    assert(inst->dfa->states[inst->current].fallback != -1);
    dest = inst->dfa->states[inst->current].fallback;
  }
  else
  {
    dest = *value;
  }
  return inst->current = dest;
}

bool dfa_acceptable(const DFAInst inst)
{
  return inst->dfa->states[inst->current].acceptable;
}

void dfa_destory_inst(DFAInst inst)
{
  free(inst);
}
