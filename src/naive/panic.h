/**
 * \file panic.h
 * \brief Provide a fail-fast assertion oriented to user.
 * \author liftA42
 * \date Dec 13, 2017
 *
 * Using this module is similiar to using `assert`, with two difference:
 * * `assert` will be disabled when `NDEBUG` macro is defined, but this module
 * will not.
 * * `assert` should fail because the mistakes made by library designer and
 * creator, while this module fails on user's fault, such as, passing invalid
 * arguments into interface functions.
 */
#ifndef LIFTA42_NAIVE_PANIC_H
#define LIFTA42_NAIVE_PANIC_H

#include <stdbool.h>

/** Exit the program immediately after printing a piece of message if assertion
fails.

Message style is recommended to all lower case, cause this leads to shorter
message, which corresponds to design.

\param assertion the judgement of whether everything is ok
\param message print out if `assertion` evaluates to false
\param ... variables fills `message` */
void panic(const bool assertion, const char *message, ...);

#endif
