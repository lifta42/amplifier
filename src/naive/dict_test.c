// Created by liftA42 on Dec 12, 2017.
#include "dict.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

static void repeat_random_access(const int repeat_time, Dict dict, int scale,
                                 int keys[], double values[])
{
  for (int inner_count = 0; inner_count < repeat_time; inner_count++)
  {
    for (int i = 0; i < scale; i++)
    {
      values[i] = (double)rand() / RAND_MAX;
      dict_put(dict, &keys[i], &values[i]);
    }
    for (int i = 0; i < scale; i++)
    {
      double *val = dict_get(dict, &keys[i], NULL);
      assert(val != NULL);
      assert(*val == values[i]);
    }
  }
}

// clang-format off
dict_gen_hash(hash_int, int)
dict_gen_equal(equal_int, int)

int main()
// clang-format on
{
  {
    Dict dict = dict_create(int, int, hash_int, equal_int);
    int key = 42, value = 43;
    dict_put(dict, &key, &value);
    int *val = dict_get(dict, &key, NULL);
    assert(*val == 43);
    key++;
    val = dict_get(dict, &key, NULL);
    assert(val == NULL);
    dict_destory(dict);
  }
  {
    const int repeat_time = 1 << 10, inner_time = 1 << 6;
    const int max_key = 2 << 16, scale_pow = 9, scale = 1 << scale_pow;
    for (int repeat_count = 0; repeat_count < repeat_time; repeat_count++)
    {
      Dict dict = dict_create(int, double, hash_int, equal_int);
      int keys[scale];
      double values[scale];
      srand(time(NULL));
      for (int i = 0; i < scale; i++)
      {
        keys[i] = rand() % (max_key / scale) + max_key / scale * i;
      }
      for (int i = 4; i < scale_pow; i++) {
        repeat_random_access(inner_time, dict, 1 << i, keys, values);
      }
      dict_destory(dict);
    }
  }
}
