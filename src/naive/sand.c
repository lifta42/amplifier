// Created by liftA42 on Dec 14, 2017.
#include "panic.h"
#include "sand.h"
#include "lang.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#endif

// the *nix implementation
// http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html
#ifndef _WIN32

struct _Sandbox
{
  SandProc *proc;
  int filedes[2][2];
  bool executed;
};

Sandbox sand_create(SandProc *proc)
{
  Sandbox box   = malloc_s(sizeof(struct _Sandbox));
  box->proc     = proc;
  box->executed = false;
  panic(pipe(box->filedes[0]) != -1, "cannot create pipe to sandbox");
  panic(pipe(box->filedes[1]) != -1, "cannot create pipe to sandbox");
  return box;
}

static void connect_pipe(int fds[2], int fileno)
{
  while ((dup2(fds[1], fileno) == -1) && (errno == EINTR))
  {
    // nothing, just waiting
  }
  close(fds[0]);
  close(fds[1]);
}

int sand_run(Sandbox box, void *payload)
{
  panic(!box->executed, "a sandbox has been executed for multiple times");
  box->executed = true;

  pid_t pid = fork();
  panic(pid != -1, "cannot create subprocess for sandbox");
  if (pid == 0)
  {
    connect_pipe(box->filedes[0], STDOUT_FILENO);
    connect_pipe(box->filedes[1], STDERR_FILENO);
    exit(box->proc(payload));
  }

  close(box->filedes[0][1]);
  close(box->filedes[1][1]);
  int status;
  waitpid(pid, &status, 0);
  return WEXITSTATUS(status);
}

int sand_read(Sandbox box, int stream, char *buffer, int buffer_size)
{
  panic(box->executed, "the read sandbox has not executed");
  int total = 0;
  while (true)
  {
    ssize_t count =
        read(box->filedes[stream][0], &buffer[total], buffer_size - total);
    panic(count >= 0, "cannot read from sandbox");
    if (count == 0)
    {
      break;
    }
    else
    {
      total += count;
    }
  }
  return total;
}

void sand_destroy(Sandbox box)
{
  close(box->filedes[0][0]);
  close(box->filedes[1][0]);
  clean(box, free);
}

// the Windows implementation
#else

// struct _Sandbox {
//
// }
something

#endif
