// Created by liftA42 on Dec 11, 2017.
#include "dfa.h"
#include "../naive/lang.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>


int main()
{
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
    int accept[] = {0, 0, 0, 0, 0,
                    1, 1, 1, 1, 1};  // could use `bool`, but ugly
    int states[] = {q0, q0, q1, q1, q1, q2, q2, q2, q2, q2};
    for (int i = 0; i < sizeof(serial) / sizeof(int); i++)
    {
      assert(dfa_send(inst, serial[i]) == states[i]);
      assert(dfa_acceptable(inst) == accept[i]);
    }

    clean(inst, dfa_destory_inst);
    clean(dfa, dfa_destroy);
  }
  {
    DFAuto dfa = dfa_create();

    DFAState q0 = dfa_extend(dfa);
    DFAState q1 = dfa_extend(dfa);
    DFAState q2 = dfa_extend(dfa);

    dfa_connect(dfa, q0, 0, q1);
    dfa_connect_fallback(dfa, q0, q2);

    DFAInst inst = dfa_freeze(dfa, q0);
    assert(dfa_send(inst, 0) == q1);
    dfa_destory_inst(inst);

    inst = dfa_freeze(dfa, q0);
    assert(dfa_send(inst, 42) == q2);
    dfa_destory_inst(inst);

    dfa_destroy(dfa);
  }
  {
    const int repeat_time = 1 << 5, inner_time = 1 << 6, jump_time = 1 << 10;
    // symbol `0` means random symbol, which shoule trigger fallback
    const int state_count = 1 << 9, symbol_range = 256;
    const int conn_count = state_count * 1 << 6;
    srand(time(NULL));
    for (int i = 0; i < repeat_time; i++)
    {
      // table[<start state>][<received symbol>] == <destined state>
      int table[state_count][symbol_range];
      for (int j = 0; j < state_count; j++)
      {
        for (int k = 0; k < symbol_range; k++)
        {
          table[j][k] = -1;
        }
      }

      DFAuto dfa = dfa_create();
      for (int j = 0; j < state_count; j++)
      {
        dfa_extend(dfa);
      }

      int remain = conn_count;
      while (remain > 0)
      {
        int start = rand() % state_count, symbol = rand() % symbol_range;
        if (table[start][symbol] != -1)
        {
          continue;
        }
        int dest = rand() % state_count;  // ok to return to start

        table[start][symbol] = dest;
        if (symbol != 0)
        {
          dfa_connect(dfa, start, symbol, dest);
        }
        else
        {
          dfa_connect_fallback(dfa, start, dest);
        }
        remain--;
      }

      for (int j = 0; j < inner_time; j++)
      {
        int curr     = rand() % state_count;
        DFAInst inst = dfa_freeze(dfa, curr);
        for (int k = 0; k < jump_time; k++)
        {
          int sym         = rand() % symbol_range;
          int expect_dest = table[curr][sym];
          if (expect_dest == -1)
          {
            // Still increase `k`, to avoid endless loop if no arrow starts from
            // `curr`.
            continue;
          }
          if (sym == 0)
          {
            sym = symbol_range + 42;  // random unknown symbol
          }

          assert(dfa_send(inst, sym) == expect_dest);
          curr = expect_dest;
        }
        clean(inst, dfa_destory_inst);
      }
      clean(dfa, dfa_destroy);
    }
  }
}
