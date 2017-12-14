// Created by liftA42 on Dec 13, 2017.
#include "panic.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

int main()
{
  {
    panic(42 == 42, "the world is running in a wrong way");
  }

#ifndef _WIN32
  {
    // http://www.microhowto.info/howto/capture_the_output_of_a_child_process_in_c.html
    int filedes[2];
    pipe(filedes);
    pid_t pid = fork();

    if (pid == 0)
    {
      while ((dup2(filedes[1], STDERR_FILENO) == -1) && (errno == EINTR))
      {
        // nothing, just waiting
      }
      close(filedes[1]);
      close(filedes[0]);
      panic(42 == 43, "this one should fail with a greeting: %s", "hello!");
    }

    int status;
    waitpid(pid, &status, 0);
    assert(WEXITSTATUS(status) == 1);

    close(filedes[1]);
    char buffer[64];
    while (true)
    {
      ssize_t count = read(filedes[0], buffer, sizeof(buffer));
      if (count == -1)
      {
        assert(errno == EINTR);
        continue;
      }
      else
      {
        break;
      }
    }
    close(filedes[0]);
    assert(strcmp(buffer, "this one should fail with a greeting: hello!\n")
           == 0);
  }
#else
  {
    // TODO
  }
#endif
}
