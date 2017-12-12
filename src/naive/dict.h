/**
 * \file dict.h
 * \brief A naive implementation of key-value dictionary.
 * \author liftA42
 * \date Dec 12, 2017
 *
 * The dictionary uses linear search on small data set, and hash table on bigger
 * one.
 *
 * A verb for hashing the keys and another verb for checking if two keys are
 * equal are required.
 */
#ifndef LIFTA42_NAIVE_DICT_H
#define LIFTA42_NAIVE_DICT_H

#include <stdbool.h>

typedef struct _Dict *Dict;

/** Accepts a key and a range as arguments, return the hashed code of `key`,
which should be between 0 (include) and `range` (exclude). */
typedef int(DictHash)(const void *key, const int range);

typedef bool(DictEqual)(const void *key1, const void *key2);

/** Create a dict with two verbs.

\param hash method for hash a key of dict
\param equal method for compare whether two keys are equal
\return The created empty dict. */
Dict dict_create(DictHash hash, DictEqual equal);

/** Naive hash method generator for primitive types. */
#define dict_gen_hash(name, Type)                                              \
  int name(const void *key, const int range)                                   \
  {                                                                            \
    Type *raw = key;                                                           \
    return (int)(*raw) % range;                                                \
  }

/** Naive equal method generator for primitive types. */
#define dict_gen_equal(name, Type)                                             \
  bool name(const void *key1, const void *key2)                                \
  {                                                                            \
    Type *val1 = *key1, *val2 = *key2;                                         \
    return *val1 == *val2;                                                     \
  }

/** Store a key-value pair in a dictionary.

If the key is present in `dict`, then its value will be substituted with
`value`, and the old value will be returned. Otherwise, a new key-value pair
will be created in `dict`, and `NULL` will be returned.

The copies of `key` and `value` will be created and stored in `dict`.

\param dict the modified dictionary
\param key the manipulated key
\param value the value of `key`
\return The old value, or `NULL` if there's no old value. */
void *dict_put(Dict dict, const void *key, const void *value);

/** Fetch a value of specified key in a dictionary.

If `key` is absent from `dict`, then `default_value` will be returned. Please
make sure that `default_value` is different from any possible value in the
dictionary.

\param dict the fetched dictionary
\param key the searched key in `dict`
\param default_value the default result
\return The value of `key`, or `default_value` if `key` is not found. */
void *dict_get(const Dict dict, const void *key, const void *default_value);

/** Release a dict and clean up its memory. */
void dict_destory(Dict dict);

#endif
