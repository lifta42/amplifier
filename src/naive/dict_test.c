// Created by liftA42 on Dec 12, 2017.
#include "dict.h"
#include <assert.h>

dict_gen_hash(hash_int, int)
dict_gen_equal(equal_int, int)

int main()
{
  {
    Dict dict = dict_create(hash_int, equal_int, sizeof(int), sizeof(int));
    int key = 42, value = 43;
    dict_put(dict, &key, &value);
    int *val = dict_get(dict, &key, NULL);
    assert(*val == 43);
    key++;
    val = dict_get(dict, &key, NULL);
    assert(val == NULL);
    dict_destory(dict);
  }
}
