// Interpreter and runtime for Futaba programming languange.
// Created by liftA42 on Nov 17, 2017.
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Error {
  UNRESOLVED_NAME = 0
};

struct _Env;
typedef void *(*PieceFunc)(void *, struct _Env *);

typedef struct {
  PieceFunc func;
  struct _Env *env;
} Piece;

Piece *piece_create(PieceFunc func, struct _Env *env) {
  Piece *piece = malloc(sizeof(Piece));
  piece->func = func;
  piece->env = env;
  return piece;
}

typedef struct {
  char *name; // zero ended
  void *magic_piece; // the entry of foreign types
} Record;

struct _Env {
  Record **records;
  int size; // varied
  int length;
  struct _Env *upper;
};
typedef struct _Env Env;

#define ENV_INIT_SIZE 16
Env *env_create(Env *upper) {
  Env *env = malloc(sizeof(Env));
  env->records = malloc(sizeof(Record *) * ENV_INIT_SIZE);
  env->size = ENV_INIT_SIZE;
  env->length = 0;
  return 0;
}
void *env_resolve(Env *env, char *name) {
  for (int i = 0; i < env->length; i++) {
    if (strcmp(env->records[i]->name, name) == 0) {
      return env->records[i]->magic_piece;
    }
  }
  if (env->upper == NULL) {
    fprintf(stderr, "unresolved name: %s\n", name);
    exit(UNRESOLVED_NAME);
  } else {
    return env_resolve(env->upper, name);
  }
}
void env_register(Env *env, char *name, void *piece) {
  if (env->length == env->size) {
    env->size *= 2;
    env->records = realloc(env, sizeof(Record *) * env->size);
  }
  Record *record = malloc(sizeof(Record));
  record->name = name;
  record->magic_piece = piece;
  env->records[env->length] = record;
}

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

#define REGISTER_INT_STRING "chosen-int"
void *internal_int(void *arg, Env *env);
Piece *parse_int(Source *source) {
  int num = 0;
  do {
    num = num * 10 + (source_fetch(source) - '0');
    if (!source_forward(source)) {
      break;
    }
  } while (isdigit(source_fetch(source)));

  int *int_piece = malloc(sizeof(int));
  *int_piece = num;
  Env *env = env_create(NULL);
  env_register(env, REGISTER_INT_STRING, int_piece);
  return piece_create(internal_int, env);
}

void *internal_int(void *arg, Env *env) {
  Piece *p = arg;
  return p->func(piece_create(internal_int, env), p->env);
}

int main() {
  //
}
