// Created by liftA42 on Dec 13, 2017.
#include "panic.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic(const bool assertion,
                const char *message, ...)
{
  if (!assertion)
  {
    va_list vl;
    va_start(vl, message);
    vfprintf(stderr, message, vl);
    va_end(vl);

    fprintf(stderr, "\n");
    exit(1);
  }
}
