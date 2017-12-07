// An interpreter for Mitsuha Programming Language implemented in C.
// Mitsuha was created by liftA42 in Nov, 2017, and the interpreter was created
// by liftA42 on Dec 4, 2017.
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// I hate SHOUT_OFF_LIKE_THIS.
#define perference_jigsaw_initial_room 32
#define perference_warehouse_initial_size 128


#ifndef NDEBUG
#define write_log(indent, message...)                                          \
  fprintf(stderr, "[debug] %*s", indent * 2, "");                              \
  fprintf(stderr, message);                                                    \
  fprintf(stderr, "\n")
#else
#define write_log(message...)
#endif


struct _Piece;
typedef void Backpack;  // backpack can be any type
typedef struct _Piece *(PieceMethod)(struct _Piece *, Backpack *);
typedef void(DropMethod)(void *);

typedef struct _Piece
{
  PieceMethod *method;
  // `BackpackDrop` may be a properer name, but that may cause a conflict if
  // an internal called `drop` exists and it requires a backpack struct.
  DropMethod *drop;
  Backpack *backpack;
  int refcount;
} Piece;

typedef struct
{
  void *memory;
  void *top;
  // I am going to use `int` instead of `size_t` at every place I should use
  // `size_t` because it looks ugly.
  int capacity;
  int allocated;  // always equals to `(top - memory)`
} Warehouse;

typedef struct
{
  Piece *pieces;  // in-place stored `Piece` array
  int capacity;
  int allocated;
  Warehouse *warehouse;
} Jigsaw;

Jigsaw *jigsaw_create()
{
  write_log(0, "creating jigsaw");
  write_log(1, "jigsaw rooms: %d", perference_jigsaw_initial_room);
  write_log(1, "warehouse size: %d", perference_warehouse_initial_size);

  Warehouse *warehouse = malloc(sizeof(Warehouse));
  assert(warehouse != NULL);
  warehouse->capacity = perference_warehouse_initial_size;
  warehouse->memory   = malloc(warehouse->capacity);
  assert(warehouse->memory != NULL);
  warehouse->allocated = 0;
  warehouse->top       = warehouse->memory;

  Jigsaw *jigsaw = malloc(sizeof(Jigsaw));
  assert(jigsaw != NULL);
  jigsaw->capacity = perference_jigsaw_initial_room;
  jigsaw->pieces   = malloc(sizeof(Piece) * jigsaw->capacity);
  assert(jigsaw->pieces != NULL);
  jigsaw->allocated = 0;
  jigsaw->warehouse = warehouse;

  write_log(0, "created jigsaw at %p", jigsaw);
  return jigsaw;
}

void debug_jigsaw(int indent, Jigsaw *jigsaw)
{
  write_log(indent, "jigsaw at %p", jigsaw);
  write_log(indent + 1, "pieces buffer: %p capacity: %d allocated: %d",
            jigsaw->pieces, jigsaw->capacity, jigsaw->allocated);
  write_log(indent + 1, "warehouse(%p) status", jigsaw->warehouse);
  write_log(indent + 2, "backpack buffer: %p, capacity: %d allocated: %d",
            jigsaw->warehouse->memory, jigsaw->warehouse->capacity,
            jigsaw->warehouse->allocated);
}

Piece *piece_allocate(Jigsaw *jigsaw, int backpack_size)
{
  write_log(0, "allocating piece with:");
  debug_jigsaw(1, jigsaw);

  assert(jigsaw->allocated < jigsaw->capacity);
  Warehouse *warehouse = jigsaw->warehouse;
  assert(warehouse->capacity - warehouse->allocated >= backpack_size);

  Piece *piece = &jigsaw->pieces[jigsaw->allocated];
  jigsaw->allocated++;
  piece->backpack = warehouse->top;
  warehouse->top += backpack_size;
  warehouse->allocated += backpack_size;

  return piece;
}

Piece *piece_initialize_impl(Jigsaw *jigsaw, PieceMethod *method,
                             DropMethod *drop, int backpack_size,
                             void *backpack)
{
  write_log(0, "initializing piece with pack_size: %d", backpack_size);
  Piece *piece  = piece_allocate(jigsaw, backpack_size);
  piece->method = method;
  piece->drop   = drop;
  memcpy(piece->backpack, backpack, backpack_size);
  piece->refcount = 0;

  return piece;
}

#define piece_initialize(jigsaw, method, drop, pack)                           \
  piece_initialize_impl(jigsaw, method, drop, sizeof(*pack), pack)

void piece_hold(Piece *holdee, Piece *holder)
{
  write_log(0, "piece %p is held by piece %p", holdee, holder);
  holdee->refcount++;
  write_log(1, "increase refcount of piece to %d", holdee->refcount);
}

void piece_giveup(Piece *piece)
{
  write_log(0, "piece %p is gave up", piece);
  assert(piece->refcount > 0);
  piece->refcount--;
  write_log(1, "decrease its refcount to %d", piece->refcount);
  if (piece->refcount == 0)
  {
    write_log(1, "no one holds this piece, discarding it");
    piece->drop(piece->backpack);
    // TODO: mark its place as usable
  }
}


Piece *apply(Piece *caller, Piece *callee)
{
  return caller->method(callee, caller->backpack);
}


typedef struct
{
  int foo;
  int bar;
} BackpackFoo;

int main(int argc, char *argv[])
{
  Jigsaw *j = jigsaw_create();
  Piece *p =
      piece_initialize(j, NULL, NULL, (&(BackpackFoo){.foo = 42, .bar = 43}));
  debug_jigsaw(0, j);
  piece_hold(p, NULL);
  // piece_giveup(p);
}
