// Created by liftA42 on Dec 13, 2017.
#include "panic.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void panic(const bool assertion, const char *message, ...)
{
  if (!assertion)
  {
    va_list vl;
    va_start(vl, message);
    vfprintf(stderr, message, vl);
    va_end(vl);
    exit(1);
  }
}
