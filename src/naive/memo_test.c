// Created by liftA42 on Dec 14, 2017.
#include "memo.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main()
{
  const int repeat_time = 1 << 10, inner_time = 1 << 6;
  const int rand_range = 256;

  srand(time(NULL));
  for (int i = 0; i < repeat_time; i++)
  {
    int init_size = 1 << 4;
    Memo memo     = memo_create(init_size);
    MemoOffset blocks[inner_time];
    for (int j = 0; j < inner_time; j++)
    {
      int size  = rand() % rand_range + sizeof(int);
      blocks[j] = memo_alloc(memo, size);
      int *m    = memo_ref(memo, blocks[j]);
      memset(m, 0, size);
      m[0] = size;
    }
    // sentry
    *(int *)memo_ref(memo, memo_alloc(memo, sizeof(int))) = -1;

    for (int j = 0; j < inner_time; j++)
    {
      char *pos = memo_ref(memo, blocks[j]);
      int size  = *(int *)pos;
      assert(size >= sizeof(int));
      pos += sizeof(int);
      for (int k = 0; k < size - sizeof(int); k++)
      {
        assert(pos[k] == 0);
      }
    }
    clean(memo, memo_destroy);
  }
}
