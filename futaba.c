// Interpreter and runtime for Futaba programming languange.
// Created by liftA42 on Nov 17, 2017.
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Error { UNRESOLVED_NAME = 0 };

// `Piece` structure
// Everything in Futaba is a `Piece`.
struct _Piece;
typedef struct _Piece *(*PieceFunc)(struct _Piece *, void *);

struct _Piece {
  PieceFunc function;
  void *backpack;
};
typedef struct _Piece Piece;

Piece *piece_create(PieceFunc func, void *pack) {
  Piece *piece = malloc(sizeof(Piece));
  piece->function = func;
  piece->backpack = pack;
  return piece;
}

Piece *internal_int(Piece *, void *);
Piece *piece_create_int(int num) {
  int *p_int = malloc(sizeof(int));
  *p_int = num;
  return piece_create(internal_int, p_int);
}

// `Table` structure
// Compile (parse) time helper for linking names and `Piece`s.
typedef struct {
  char *name; // zero ended
  Piece *piece;
} Record;

typedef struct _Table {
  Record **records;
  int size; // varied
  int length;
  struct _Table *upper;
} Table;

#define TABLE_INIT_SIZE 16
Table *table_create(Table *upper) {
  Table *table = malloc(sizeof(Table));
  table->records = malloc(sizeof(Record *) * TABLE_INIT_SIZE);
  table->size = TABLE_INIT_SIZE;
  table->length = 0;
  return table;
}
Piece *table_resolve(Table *table, char *name) {
  for (int i = 0; i < table->length; i++) {
    if (strcmp(table->records[i]->name, name) == 0) {
      return table->records[i]->piece;
    }
  }
  if (table->upper == NULL) {
    fprintf(stderr, "unresolved name: %s\n", name);
    exit(UNRESOLVED_NAME);
  } else {
    return table_resolve(table->upper, name);
  }
}
void table_register(Table *table, char *name, Piece *piece) {
  if (table->length == table->size) {
    table->size *= 2;
    table->records = realloc(table, sizeof(Record *) * table->size);
  }
  Record *record = malloc(sizeof(Record));
  record->name = name;
  record->piece = piece;
  table->records[table->length] = record;
  table->length++;
}

// Part 1, parser
// `Source` structure
// A `Source` represents a source code file.
typedef struct {
  char *source;
  int length;
  int current;
  int line;
  int column;
} Source;

char source_fetch(Source *s) { return s->source[s->current]; }
bool source_forward(Source *s) {
  if (s->source[s->current] == '\n') {
    s->line++;
    s->column = 0;
  } else {
    s->column++;
  }
  s->current++;
  return s->current != s->length;
}
Source *source_create(char *s, int len) {
  Source *source = malloc(sizeof(Source));
  source->source = s;
  source->length = len;
  source->current = 0;
  source->line = source->column = 1;
  return source;
}

Piece *parse_int(Source *source) {
  int num = 0;
  do {
    num = num * 10 + (source_fetch(source) - '0');
    if (!source_forward(source)) {
      break;
    }
  } while (isdigit(source_fetch(source)));
  return piece_create_int(num);
}

// Part 2, apply
void *apply(Piece *caller, Piece *callee) {
  return caller->function(callee, caller->backpack);
}

// Part 3, internal `Piece`s
Piece *internal_int(Piece *callee, void *backpack) {
  int *p_num = backpack;
  return apply(callee, piece_create_int(*p_num));
}

int main() {
  //
}
