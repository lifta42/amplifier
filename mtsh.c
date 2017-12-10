// An interpreter for Mitsuha Programming Language implemented in C.
// Mitsuha was created by liftA42 in Nov, 2017, and the interpreter was created
// by liftA42 on Dec 4, 2017.
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// I hate to SHOUT_OFF_LIKE_THIS.
// Use `const int` instead of `#define` so that clang-format will align them.
const int perference_jigsaw_initial_room             = 32;
const int perference_warehouse_initial_size          = 128;
const int perference_debug_source_content_max_length = 32;


#ifndef NDEBUG
#define write_log(indent, message...)                                          \
  fprintf(stderr, "[debug] %*s", (indent)*2, "");                              \
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

Jigsaw *jigsaw_create()
{
  write_log(0, "creating jigsaw");

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

  write_log(0, "created jigsaw:");
  debug_jigsaw(1, jigsaw);
  return jigsaw;
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

Piece *piece_create_impl(Jigsaw *jigsaw, PieceMethod *method, DropMethod *drop,
                         int backpack_size, void *backpack)
{
  write_log(0, "initializing piece with pack_size: %d", backpack_size);
  Piece *piece  = piece_allocate(jigsaw, backpack_size);
  piece->method = method;
  piece->drop   = drop;
  memcpy(piece->backpack, backpack, backpack_size);
  piece->refcount = 0;

  return piece;
}

#define piece_create(jigsaw, method, drop, pack)                               \
  piece_create_impl(jigsaw, method, drop, sizeof(*pack), pack)

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
    if (piece->drop != NULL)
    {
      piece->drop(piece->backpack);
    }
    // TODO: mark its place as usable
  }
}


Piece *apply(Piece *caller, Piece *callee)
{
  return caller->method(callee, caller->backpack);
}


typedef struct
{
  char *buffer;
  int offset;
  int line;
  int column;
  int length;
} Source;

Source *source_create(char *buffer, int length)
{
  Source *source = malloc(sizeof(Source));
  source->buffer = buffer;
  source->length = length;
  source->offset = 0;
  source->line = source->column = 1;
  return source;
}

bool source_forward(Source *source)
{
  if (source->offset == source->length)
  {
    return false;
  }
  if (source->buffer[source->offset] == '\n')
  {
    source->line++;
    source->column = 1;
  }
  else
  {
    source->column++;
  }
  source->offset++;
  return true;
}

// Fetching from `source` does not have to invoking this.
char source_peek(Source *source)
{
  assert(source->offset >= 0 && source->offset < source->length);
  return source->buffer[source->offset];
}

typedef struct _Register
{
  char *name;
  int name_length;
  Piece *piece;
  struct _Register *upper;
} Register;

void debug_source(int indent, Source *source)
{
  write_log(indent, "source(%p) {", source);
  write_log(indent + 1, "line: %d, column: %d,", source->line, source->column);

  int remain_length = source->length - source->offset;
  int log_length    = perference_debug_source_content_max_length;
  char *yadda       = "...";
  if (remain_length < log_length)
  {
    log_length = remain_length;
    yadda      = "";
  }
  write_log(indent + 1, "content: \"%.*s%s\"", log_length, source->buffer,
            yadda);
  write_log(indent, "}");
}

Register *register_create_impl(char *name, int name_length, Piece *piece,
                               Register *upper)
{
  Register *reg    = malloc(sizeof(Register));
  reg->name        = name;
  reg->name_length = name_length;
  reg->piece       = piece;
  reg->upper       = upper;
  return reg;
}

// This `name` must be a string literal.
#define register_create(name, piece, upper)                                    \
  register_create_impl(name, sizeof(name), piece, upper)

Piece *register_resolve(Register *reg, Source *source)
{
  write_log(0, "resolving name in register %p", reg);
  Piece *match_piece = NULL;
  int match_length   = 0;
  for (Register *reg_iter = reg; reg_iter != NULL; reg_iter = reg_iter->upper)
  {
    int max_length = source->length - source->offset;
    max_length =
        max_length > reg_iter->name_length ? reg_iter->name_length : max_length;
    if (strncmp(&source->buffer[source->offset], reg_iter->name, max_length)
        == 0)
    {
      if (reg_iter->name_length > match_length)
      {
        match_piece  = reg_iter->piece;
        match_length = reg_iter->name_length;
      }
    }
  }
  if (match_piece == NULL)
  {
    write_log(1, "fail to resolve name for source");
    debug_source(2, source);
    return NULL;
  }
  else
  {
    write_log(1, "resolved piece %p with matched name %.*s", match_piece,
              match_length, &source->buffer[source->offset]);
    for (int i = 0; i < match_length; i++)
    {
      source_forward(source);
    }
    return match_piece;
  }
}

Piece *internal_call(Piece *callee, Backpack *backpack) {
  return NULL;
}

typedef struct {
  Piece *caller;
  Piece *callee;
} BackpackCall;

Piece *parse_sentence_impl(Source *source, Register *reg, Jigsaw *jigsaw,
                           Piece *covered)
{
  while (isspace(source_peek(source)))
  {
    if (!source_forward(source))
    {
      return NULL;
    }
  }

  Piece *piece = register_resolve(reg, source);
  if (piece == NULL)
  {
    return NULL;
  }

  Piece *call_piece =
      piece_create(jigsaw, internal_call, NULL,
                   (&(BackpackCall){.caller = covered, .callee = piece}));
  piece_hold(call_piece, covered);
  piece_hold(call_piece, piece);
  return parse_sentence_impl(source, reg, jigsaw, call_piece);
}

Piece *parse_sentence(Source *source, Register *reg, Jigsaw *jigsaw)
{
  return parse_sentence_impl(source, reg, jigsaw, NULL);
}

int main(int argc, char *argv[])
{
  //
}
