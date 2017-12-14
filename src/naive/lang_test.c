// Created by liftA42 on Dec 14, 2017.
#include "lang.h"
#include "sand.h"
#include <assert.h>
#include <stdlib.h>

typedef struct
{
  struct
  {
    int foo_i;
    double foo_d;
  } foo;
  struct
  {
    float bar_f;
    void *bar_p;
  } * bar;
} Foo;

size_t align(size_t s)
{
  // May fail on 32-bit machine.
  // who cares
  return ((s - 1) / 8 + 1) * 8;
}

int fail_alloc(void *payload)
{
  malloc_s(0);
  return 0; // trivial
}

int fail_alloc_2(void *payload)
{
  malloc_s((1 << 31) + (1 << 30));
  return 0; // trivial
}

int main()
{
  assert(sizeof(field(Foo, foo)) == align(sizeof(int) + sizeof(double)));
  assert(sizeof(*field(Foo, bar)) == align(sizeof(float) + sizeof(void *)));

  void *mem = malloc_s(64);
  assert(mem != NULL);
  clean(mem, free);
  assert(mem == NULL);

  Sandbox box = sand_create(fail_alloc);
  assert(sand_run(box, NULL) == 1);
  clean(box, sand_destroy);

  box = sand_create(fail_alloc_2);
  assert(sand_run(box, NULL) == 1);
  clean(box, sand_destroy);
}
