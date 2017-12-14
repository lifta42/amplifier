// Created by liftA42 on Dec 13, 2017.
#include "panic.h"
#include "sand.h"
#include "lang.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

int panic_error(void *payload)
{
  panic(42 == 43, "this one should fail with a greeting: %s", "hello!");
  return 0;  // trivial
}

int main()
{
  {
    panic(42 == 42, "the world is running in a wrong way");
  }
  {
    Sandbox box = sand_create(panic_error);
    assert(sand_run(box, NULL) == 1);
    char buffer[64];
    sand_read(box, 1, buffer, sizeof(buffer));
    assert(strcmp(buffer, "this one should fail with a greeting: hello!\n")
           == 0);
    clean(box, sand_destroy);
  }
}
