// Interpreter and runtime for Futaba programming languange.
// Created by liftA42 on Nov 17, 2017.
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Error {
  UNRESOLVED_NAME = 0
};

struct _Env;
typedef void *(*Piece)(void *, struct _Env *);

typedef struct {
  char *name; // zero ended
  Piece piece;
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
Piece env_resolve(Env *env, char *name) {
  for (int i = 0; i < env->length; i++) {
    if (strcmp(env->records[i]->name, name) == 0) {
      return env->records[i]->piece;
    }
  }
  if (env->upper == NULL) {
    fprintf(stderr, "unresolved name: %s\n", name);
    exit(UNRESOLVED_NAME);
  } else {
    return env_resolve(env->upper, name);
  }
}
void env_register(Env *env, char *name, Piece piece) {
  if (env->length == env->size) {
    env->size *= 2;
    env->records = realloc(env, sizeof(Record *) * env->size);
  }
  Record *record = malloc(sizeof(Record));
  record->name = name;
  record->piece = piece;
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

int main() {
  //
}
