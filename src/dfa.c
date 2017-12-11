// Created by liftA42 on Dec 11, 2017.
#include "dfa.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct _DFAuto
{
  struct
  {
    struct
    {
      DFASymbol symbol;
      DFAState state;
    } * records;
    int record_length;
    int record_capacity;
    DFAState fallback;
    bool acceptable;
  } * states;
  int state_count;
  int state_capacity;
};

// https://stackoverflow.com/questions/35237503/sizeof-anonymous-nested-struct
#define sizeof_field(s, m) (sizeof((((s *)0)->m)))

DFAuto dfa_create(void)
{
  struct _DFAuto *dfa = malloc(sizeof(struct _DFAuto));
  assert(dfa != NULL);
  dfa->state_capacity = 16;
  dfa->states =
      malloc(sizeof_field(struct _DFAuto, states) * dfa->state_capacity);
  assert(dfa->states != NULL);
  dfa->state_count = 0;
  return dfa;
}

static void extend_when_necessary(void *memory, int length, int *capacity,
                                  size_t unit)
{
  if (length == *capacity)
  {
    *capacity *= 2;
    memory = realloc(memory, unit * *capacity);
    assert(memory != NULL);
  }
}

DFAState dfa_extend(DFAuto dfa)
{
  extend_when_necessary(dfa->states, dfa->state_count, &dfa->state_capacity,
                        sizeof_field(struct _DFAuto, states));
  // I am regretting for my laziness, that not willing to give the inner struct
  // a name.
  dfa->states[dfa->state_count].record_capacity = 16;
  dfa->states[dfa->state_count].records =
      malloc(sizeof_field(struct _DFAuto, states->records)
             * dfa->states[dfa->state_count].record_capacity);
  assert(dfa->states[dfa->state_count].records != NULL);
  dfa->states[dfa->state_count].record_length = 0;
  dfa->states[dfa->state_count].fallback      = -1;  // no fallback by default
  dfa->states[dfa->state_count].acceptable    = false;
  return dfa->state_count++;
}

void dfa_connect(DFAuto dfa, const DFAState from, const DFASymbol with,
                 const DFAState to)
{
  int state_length = dfa->states[from].record_length;
  extend_when_necessary(dfa->states[from].records, state_length,
                        &dfa->states[from].record_capacity,
                        sizeof_field(struct _DFAuto, states->records));

  for (int i = 0; i < state_length; i++)
  {
    assert(dfa->states[from].records[i].symbol != with);
  }
  dfa->states[from].records[state_length].symbol = with;
  dfa->states[from].records[state_length].state  = to;
  dfa->states[from].record_length++;
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
    free(dfa->states[i].records);
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
  struct _DFAInst *inst = malloc(sizeof(struct _DFAInst));
  assert(inst != NULL);
  // Cannot assign directly with `inst->dfa = dfa`, because const member cannot
  // be assigned.
  memcpy(inst, (&(struct _DFAInst){.dfa = dfa, .current = start}),
         sizeof(struct _DFAInst));
  return inst;
}

void dfa_send(DFAInst inst, const DFASymbol symbol)
{

  for (int i = 0; i < inst->dfa->states[inst->current].record_length; i++)
  {
    if (inst->dfa->states[inst->current].records[i].symbol == symbol) {
      inst->current = inst->dfa->states[inst->current].records[i].state;
      return;
    }
  }
  assert(false);  // no valid transition
}

bool dfa_acceptable(const DFAInst inst) {
  return inst->dfa->states[inst->current].acceptable;
}

void dfa_destory_inst(DFAInst inst) {
  free(inst);
}
