/**
 * \file memo.h
 * \brief Memory manager for simple allocation usage.
 * \author liftA42
 * \date Dec 14, 2017
 *
 * The submodule is helpful when you want to allocate tons of memory piece by
 * piece. It can reduce the calling of `malloc` by allocating block of memory
 * and passing it to you when you ask.
 */
#ifndef LIFTA42_NAIVE_MEMO_H
#define LIFTA42_NAIVE_MEMO_H

#include "lang.h"

typedef struct _Memo *Memo;
typedef size_t MemoOffset;

/** Create a `memo` with a specified initial size.

The size of a `memo` will double when it runs out of memory.

\param initial_size how much memory do you need at start
\return The created `memo` object. */
Memo memo_create(size_t initial_size);

/** Allocate a piece of memory inside a `memo`.

There's no "real" `realloc` for `Memo`, just discard the old memory and ask a
new one, as rare as possible.

An indirect offset is involved, because the address may vary when the `memo` is
extended.

\param memo the manager you ash memory from
\param size the memory size you need
\return Your required memory's offset, which can be used to refer the real
memory */
MemoOffset memo_alloc(Memo memo, size_t size);

/** Get the memory address with offset.

\param memo where your data stores
\param offset the offset you get when allocating
\return The real address of data. */
void *memo_ref(Memo memo, MemoOffset offset);

/** Release a `memo`.

\param memo the released memo */
void memo_destroy(Memo memo);

#endif
