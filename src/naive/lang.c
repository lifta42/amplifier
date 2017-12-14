// Created by liftA42 on Dec 14, 2017.
#include "lang.h"
#include <stdlib.h>

static void *alloc(size_t size, void *pre, const char *func)
{
  panic(size != 0, "pass 0 to %s", func);
  void *mem = pre == NULL ? malloc(size) : realloc(pre, size);
  panic(mem != NULL, "%s returns NULL", func);
  return mem;
}

void *malloc_s(size_t size)
{
  return alloc(size, NULL, "malloc");
}

void *realloc_s(void *who, size_t size)
{
  return alloc(size, who, "realloc");
}
