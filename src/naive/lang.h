/**
 * \file lang.h
 * \brief Some "patch" on C language
 * \author liftA42
 * \date Dec 14, 2017
 *
 * Several useful misc are included.
 * \warning The name of member of this submodule has no prefix.
 */
#ifndef LIFTA42_NAIVE_LANG_H
#define LIFTA42_NAIVE_LANG_H

#include <stdbool.h>
#include <stddef.h>

#include "panic.h"

/** Get a member of structure.

Useful when getting size of a anonymous nested struct size.
\param s the structure type
\param m the member name */
// https://stackoverflow.com/questions/35237503/sizeof-anonymous-nested-struct
#define field(s, m) (((s *)0)->m)

/** Safe version of `malloc`.

This would get panic when allocating is failed or `0` is passed.
\param size how much memory do you want
\return The pointer to newly allocated memory on the heap. */
void *malloc_s(size_t size);

/** Safe version of `realloc`.

\param who the memory that need to expand
\param size the new size of `who`
\return the expanded memory, whose head is the same with original `who`. */
void *realloc_s(void *who, size_t size);

/** Helper for setting freed pointer to `NULL`. */
#define clean(object, cleaner)                                                 \
  cleaner(object);                                                             \
  (object) = NULL;

#endif
