// Created by liftA42 on Dec 14, 2017.
#include "sand.h"
#include "lang.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int work(void *payload) {
  char *name = payload;
  printf("hello, %s!", name);
  fprintf(stderr, "this is a error message");
  return 42;
}

int main()
{
#ifndef _WIN32
  Sandbox box = sand_create(work);
  char *name = "liftA42";
  assert(sand_run(box, name) == 42);
  char buffer[64];
  sand_read(box, 0, buffer, sizeof(buffer));
  assert(strcmp(buffer, "hello, liftA42!") == 0);
  sand_read(box, 1, buffer, sizeof(buffer));
  assert(strcmp(buffer, "this is a error message") == 0);
  clean(box, sand_destroy);
#endif
}
