// Created by liftA42 on Dec 14, 2017.
#include "memo.h"
#include <stdlib.h>

struct _Memo
{
  void *memory;
  size_t used;  // always equal to `current - memory`
  size_t allocated;
};

static size_t align_impl(size_t s)
{
  return s == 1ul ? 1ul : align_impl(s >> 1) << 1;
}

static size_t align(size_t s)
{
  return align_impl(s - 1) << 1;
}

Memo memo_create(size_t initial_size)
{
  panic(initial_size >= 16, "the initial size passed to memo is too small");
  Memo memo       = malloc_s(sizeof(struct _Memo));
  memo->memory    = malloc_s(initial_size);
  memo->used      = 0;
  memo->allocated = align(initial_size);
  return memo;
}

MemoOffset memo_alloc(Memo memo, size_t size)
{
  panic(size != 0, "passing 0 to Memo allocator");
  bool extend = false;
  while (memo->used + size > memo->allocated)
  {
    extend = true;
    memo->allocated *= 2;
  }
  if (extend)
  {
    memo->memory = realloc_s(memo->memory, memo->allocated);
  }
  size_t offset = memo->used;
  memo->used += size;
  return offset;
}

void *memo_ref(Memo memo, MemoOffset offset)
{
  panic(offset < memo->used, "invalid offset is passed to memo");
  return &memo->memory[offset];
}

void memo_destroy(Memo memo)
{
  clean(memo->memory, free);
  clean(memo, free);
}
