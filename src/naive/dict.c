// Created by liftA42 on Dec 12, 2017.
#include "dict.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

const int dict_linear_limit = 16;
const int dict_hash_initial = 128;

struct _Dict
{
  enum
  {
    DictTypeLinear,
    DictTypeHash
  } type;

  struct
  {
    void *key;
    void *value;
  } * data;

  int length;
  int capacity;

  DictHash *hash;
  DictEqual *equal;

  size_t key_size;
  size_t value_size;
};

#define sizeof_field(s, m) (sizeof((((s *)0)->m)))
#define sizeof_field_p(s, pm) (sizeof(*(((s *)0)->pm)))

// WARNING: Only be able to be called after `capacity` field is set properly.
static void initialize_data(Dict dict)
{
  size_t data_size = sizeof_field_p(struct _Dict, data) * dict->capacity;
  dict->data       = malloc(data_size);
  assert(dict->data != NULL);
  // Make sure the `key` in all free rooms in `hash_dict->data` is `NULL`.
  // What if `NULL` is not `0`?
  memset(dict->data, (int)NULL, data_size);
}

Dict dict_create(DictHash *hash, DictEqual *equal, size_t key_size,
                 size_t value_size)
{
  Dict dict = malloc(sizeof(struct _Dict));
  assert(dict != NULL);
  dict->hash     = hash;
  dict->equal    = equal;
  dict->type     = DictTypeLinear;
  dict->capacity = dict_linear_limit;
  initialize_data(dict);
  dict->length     = 0;
  dict->key_size   = key_size;
  dict->value_size = value_size;

  return dict;
}

static void covert(Dict dict);
static void extend(Dict dict);

static int quad_hash(int h, int i, int m)
{
  // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
  return (h + i * (i + 1) / 2) % m;
}

void dict_put(Dict dict, const void *key, const void *value)
{
  assert(key != NULL);
  if (dict->length == dict->capacity)
  {
    if (dict->type == DictTypeLinear)
    {
      covert(dict);
    }
    else
    {
      extend(dict);
    }
  }

  int write_pos;
  if (dict->type == DictTypeLinear)
  {
    for (int i = 0; i < dict->length; i++)
    {
      if (dict->equal(dict->data[i].key, key))
      {
        void *old = dict->data[i].value;
        memcpy(dict->data[i].value, value, dict->value_size);
        return;
      }
    }
    write_pos = dict->length;
  }
  else
  {
    int hash = dict->hash(key, dict->capacity);
    for (int i = 0; i < dict->capacity; i++)
    {
      int index = quad_hash(hash, i, dict->capacity);
      if (dict->data[index].key == NULL)
      {
        write_pos = index;
        break;
      }
      // Same logic as above. Consider to extract it.
      else if (dict->equal(dict->data[index].key, key))
      {
        void *old = dict->data[index].value;
        memcpy(dict->data[index].value, value, dict->value_size);
        return;
      }
    }
    // It is impossible that neither the data table is full, nor any valid room
    // for new pair is found.
    assert(false);
  }

  // The key is not found in data table, create a new one at proper place.
  dict->data[write_pos].key = malloc(dict->key_size);
  assert(dict->data[write_pos].key != NULL);
  memcpy(dict->data[write_pos].key, key, dict->key_size);
  dict->data[write_pos].value = malloc(dict->value_size);
  assert(dict->data[write_pos].value != NULL);
  memcpy(dict->data[write_pos].value, value, dict->value_size);
  dict->length++;
}

static void rebuild_dict(const int type, const int capacity, Dict dict)
{
  Dict rebuilt = malloc(sizeof(struct _Dict));
  assert(rebuilt != NULL);
  memcpy(rebuilt, dict, sizeof(struct _Dict));
  rebuilt->type     = type;
  rebuilt->capacity = capacity;
  initialize_data(rebuilt);

  for (int i = 0; i < dict->length; i++)
  {
    // not reuse `dict_get`, possibly should
    dict_put(rebuilt, dict->data[i].key, dict->data[i].value);
  }
  memcpy(dict, rebuilt, sizeof(struct _Dict));
}

static void extend(Dict dict)
{
  rebuild_dict(DictTypeHash, dict->capacity * 2, dict);
}

static void covert(Dict dict)
{
  rebuild_dict(DictTypeHash, dict_hash_initial, dict);
}

void *dict_get(const Dict dict, const void *key, void *fail)
{
  if (dict->type == DictTypeLinear)
  {
    for (int i = 0; i < dict->length; i++)
    {
      if (dict->equal(dict->data[i].key, key))
      {
        return dict->data[i].value;
      }
    }
    return fail;
  }
  else
  {
    int hash = dict->hash(key, dict->capacity);
    for (int i = 0; i < dict->capacity; i++)
    {
      int index = quad_hash(hash, i, dict->capacity);
      if (dict->equal(dict->data[index].key, key))
      {
        return dict->data[index].value;
      }
    }
    return fail;
  }
}

void dict_destory(Dict dict)
{
  for (int i = 0; i < dict->capacity; i++)
  {
    if (dict->data[i].key != NULL)
    {
      free(dict->data[i].key);
      free(dict->data[i].value);
    }
  }
  free(dict->data);
  free(dict);
}
