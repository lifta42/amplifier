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

Dict dict_create_impl(DictHash *hash, DictEqual *equal, size_t key_size,
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

static void extend(Dict dict);

static int quad_hash(int h, int i, int m)
{
  // https://en.wikipedia.org/wiki/Quadratic_probing#Quadratic_function
  return (h + i * (i + 1) / 2) % m;
}

static int find_key(const Dict dict, const void *key, int *fail_at)
{
  if (dict->type == DictTypeLinear)
  {
    for (int i = 0; i < dict->length; i++)
    {
      if (dict->equal(dict->data[i].key, key))
      {
        return i;
      }
    }
    if (fail_at != NULL)
    {
      *fail_at = dict->length == dict->capacity ? -1 : dict->length;
    }
    return -1;
  }
  else
  {
    int hash = dict->hash(key, dict->capacity);
    for (int i = 0; i < dict->capacity; i++)
    {
      int index = quad_hash(hash, i, dict->capacity);
      if (dict->data[index].key == NULL)
      {
        *fail_at = index;
        return -1;
      }
      if (dict->equal(dict->data[index].key, key))
      {
        return index;
      }
    }
    *fail_at = -1;
    return -1;
  }
}

void dict_put(Dict dict, const void *key, const void *value)
{
  assert(key != NULL);
  if (dict->length
      == (dict->type == DictTypeLinear ? dict->capacity
                                       : dict->capacity / 4 * 3))
  {
    extend(dict);
  }

  int write_pos;
  int found = find_key(dict, key, &write_pos);
  if (found != -1)
  {
    void *old = dict->data[found].value;
    memcpy(dict->data[found].value, value, dict->value_size);
  }
  else
  {
    // The key is not found in data table, create a new one at proper place.
    if (write_pos == -1)
    {
      extend(dict);
      dict_put(dict, key, value);
    }
    else
    {
      dict->data[write_pos].key = malloc(dict->key_size);
      assert(dict->data[write_pos].key != NULL);
      memcpy(dict->data[write_pos].key, key, dict->key_size);
      dict->data[write_pos].value = malloc(dict->value_size);
      assert(dict->data[write_pos].value != NULL);
      memcpy(dict->data[write_pos].value, value, dict->value_size);
      dict->length++;
    }
  }
}

static void rebuild_dict(const int type, const int capacity, Dict dict)
{
  Dict rebuilt = malloc(sizeof(struct _Dict));
  assert(rebuilt != NULL);
  memcpy(rebuilt, dict, sizeof(struct _Dict));
  rebuilt->type     = type;
  rebuilt->length   = 0;
  rebuilt->capacity = capacity;
  initialize_data(rebuilt);

  if (dict->type == DictTypeLinear)
  {
    for (int i = 0; i < dict->length; i++)
    {
      dict_put(rebuilt, dict->data[i].key, dict->data[i].value);
    }
  }
  else
  {
    for (int i = 0; i < dict->capacity; i++)
    {
      if (dict->data[i].key != NULL)
      {
        dict_put(rebuilt, dict->data[i].key, dict->data[i].value);
      }
    }
  }
  memcpy(dict, rebuilt, sizeof(struct _Dict));
}

static void extend(Dict dict)
{
  rebuild_dict(dict->capacity * 2 < dict_hash_initial ? DictTypeLinear
                                                      : DictTypeHash,
               dict->capacity * 2, dict);
}

void *dict_get(const Dict dict, const void *key, void *fail)
{
  int pos = find_key(dict, key, NULL);
  return pos != -1 ? dict->data[pos].value : fail;
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
