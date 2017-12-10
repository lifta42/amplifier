The Diode Programming Language

Founded by liftA42 on Oct 30, 2017. Licensed under MIT.

This is a language focus on speed. The basic component of Diode, called
procedure, is never returned to its caller, which would be convert to `jmp` in
underlying assembly easier than normal languages. The usage of stack memory may
be optimized as well.
