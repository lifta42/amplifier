// Interpreter and runtime for Futaba programming languange.
// Created by liftA42 on Nov 17, 2017.
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Part 0, preparation
enum Error {
  ERROR_UNRESOLVED_NAME = 1,
  ERROR_CANNOT_OPEN_FILE,
  ERROR_NO_ARGV,
  ERROR_UNRECOGNIZED_SYMBOL
};

#define SYNTAX_SENTENCE_END '.'
#define SYNTAX_LAMBDA_HEAD '?'
#define SYNTAX_SENTENCT_BREAK ','

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

Piece *internal_self(Piece *, void *);

#define PIECE_CREATE_GENERATE(type)                                            \
  Piece *piece_create_##type(type val) {                                       \
    type *p = malloc(sizeof(type));                                            \
    *p = val;                                                                  \
    return piece_create(internal_self, p);                                     \
  }

PIECE_CREATE_GENERATE(int)
PIECE_CREATE_GENERATE(bool)

typedef struct {
  Piece *caller;
  Piece *callee;
} BackpackCall;

Piece *internal_call(Piece *, void *);

Piece *piece_create_call(Piece *caller, Piece *callee) {
  BackpackCall *backpack = malloc(sizeof(BackpackCall));
  backpack->caller = caller;
  backpack->callee = callee;
  return piece_create(internal_call, backpack);
}

// Part 1, parser
// `Table` structure
// Compile (parse) time helper for linking names and `Piece`s.
typedef struct {
  char *name;
  int name_len;
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
  table->upper = upper;
  return table;
}

Piece *table_resolve(Table *table, char *name, int name_len) {
  if (table == NULL) {
    return NULL;
  }
  for (int i = 0; i < table->length; i++) {
    if (name_len != table->records[i]->name_len) {
      continue;
    }
    if (strncmp(table->records[i]->name, name, name_len) == 0) {
      return table->records[i]->piece;
    }
  }
  return table_resolve(table->upper, name, name_len);
}

void table_register(Table *table, char *name, int name_len, Piece *piece) {
  if (table->length == table->size) {
    table->size *= 2;
    table->records = realloc(table, sizeof(Record *) * table->size);
  }
  Record *record = malloc(sizeof(Record));
  record->name = name;
  record->name_len = name_len;
  record->piece = piece;
  table->records[table->length] = record;
  table->length++;
}

// `Source` structure
// A `Source` represents a source code file.
typedef struct {
  char *file_name;
  char *source;
  int length;
  int current;
  int line;
  int column;
} Source;

char source_fetch(Source *s) {
  return s->current == s->length ? EOF : s->source[s->current];
}

bool source_forward(Source *s) {
  if (s->current < s->length) {
    if (s->source[s->current] == '\n') {
      s->line++;
      s->column = 0;
    } else {
      s->column++;
    }
    s->current++;
  }
  return s->current != s->length;
}

Source *source_create(char *s, int len, char *file_name) {
  Source *source = malloc(sizeof(Source));
  source->source = s;
  source->file_name = file_name;
  source->length = len;
  source->current = 0;
  source->line = source->column = 1;
  return source;
}

// parser utils
char *parse_cover_name(Source *source, int *name_len) {
  char *name = &source->source[source->current];
  *name_len = 0;
  do {
    source_forward(source);
    (*name_len)++;
  } while (isgraph(source_fetch(source)) &&
           source_fetch(source) != SYNTAX_SENTENCE_END &&
           source_fetch(source) != SYNTAX_SENTENCT_BREAK);
  return name;
}

#define PARSE_GRUMBLE(source, message...)                                      \
  fprintf(stderr, message);                                                    \
  fprintf(stderr, "%s:%d:%d\n", source->file_name, source->line,               \
          source->column);

Piece *parse_aggregate_call(Piece *acc, Piece *item) {
  return acc == NULL ? item : piece_create_call(acc, item);
}

// parser components
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

Piece *parse_name(Source *source, Table *table) {
  int name_len;
  char *name = parse_cover_name(source, &name_len);
  Piece *piece = table_resolve(table, name, name_len);
  if (piece == NULL) {
    PARSE_GRUMBLE(source, "unresolved name \"%.*s\" near ", name_len, name);
    exit(ERROR_UNRESOLVED_NAME);
  }
  return piece;
}

Piece *parse_sentence(Source *, Table *);

typedef struct {
  Piece *body;
  Piece **arg;
} BackpackLambda;

Piece *internal_lambda(Piece *, void *);
Piece *internal_argument(Piece *, void *);

Piece *parse_lambda(Source *source, Table *table) {
  source_forward(source);

  BackpackLambda *backpack = malloc(sizeof(BackpackLambda));
  backpack->arg = malloc(sizeof(Piece *));
  *backpack->arg = NULL;

  int name_len;
  char *name = parse_cover_name(source, &name_len);
  Table *t = table_create(table);
  table_register(table, name, name_len,
                 piece_create(internal_argument, backpack->arg));

  backpack->body = parse_sentence(source, table);
  return piece_create(internal_lambda, backpack);
}

Piece *parse_piece(Source *source, Table *table) {
  if (isdigit(source_fetch(source))) {
    return parse_int(source);
  } else if (source_fetch(source) == '?') {
    return parse_lambda(source, table);
  } else if (isgraph(source_fetch(source))) {
    return parse_name(source, table);
  } else {
    PARSE_GRUMBLE(source, "unrecognized symbol near \'0x%x\' at ",
                  source_fetch(source) & 0xff);
    exit(ERROR_UNRECOGNIZED_SYMBOL);
  }
}

Piece *parse_sentence_impl(Source *source, Table *table, Piece *big,
                           Piece *small) {
  while (isspace(source_fetch(source))) {
    source_forward(source);
  }

  if (source_fetch(source) == SYNTAX_SENTENCE_END ||
      source_fetch(source) == EOF) {
    source_forward(source);
    return parse_aggregate_call(big, small);
  } else if (source_fetch(source) == SYNTAX_SENTENCT_BREAK) {
    source_forward(source);
    return parse_sentence_impl(source, table, parse_aggregate_call(big, small),
                               NULL);
  }

  return parse_sentence_impl(
      source, table, big,
      parse_aggregate_call(small, parse_piece(source, table)));
}

Piece *parse_sentence(Source *source, Table *table) {
  return parse_sentence_impl(source, table, NULL, NULL);
}

// Part 2, apply
Piece *apply(Piece *caller, Piece *callee) {
  return caller->function(callee, caller->backpack);
}

// Part 3, internals
Piece *internal_self(Piece *callee, void *backpack) {
  return apply(callee, piece_create(internal_self, backpack));
}

Piece *internal_call(Piece *callee, void *backpack) {
  BackpackCall *pack = backpack;
  return apply(apply(pack->caller, pack->callee), callee);
}

Piece *internal_lambda(Piece *callee, void *backpack) {
  BackpackLambda *pack = backpack;
  *pack->arg = callee;
  return pack->body;
}

Piece *internal_argument(Piece *callee, void *backpack) {
  Piece **pack = backpack;
  return apply(*pack, callee);
}

Piece *internal_put(Piece *callee, void *backpack) {
  int *p_char = callee->backpack;
  putchar(*p_char);
  return piece_create(internal_self, NULL);
}

#define INTERNAL_GENERATE_2(name, op, type)                                    \
  Piece *internal_##name##_2(Piece *, void *);                                 \
                                                                               \
  Piece *internal_##name(Piece *callee, void *backpack) {                      \
    return piece_create(internal_##name##_2, callee->backpack);                \
  }                                                                            \
                                                                               \
  struct _Backpack_##name##_2 {                                                \
    int i1;                                                                    \
    int i2;                                                                    \
  };                                                                           \
                                                                               \
  Piece *internal_##name##_3(Piece *, void *);                                 \
                                                                               \
  Piece *internal_##name##_2(Piece *callee, void *backpack) {                  \
    int *i1 = backpack, *i2 = callee->backpack;                                \
    struct _Backpack_##name##_2 *pack =                                        \
        malloc(sizeof(struct _Backpack_##name##_2));                           \
    pack->i1 = *i1;                                                            \
    pack->i2 = *i2;                                                            \
    return piece_create(internal_##name##_3, pack);                            \
  }                                                                            \
                                                                               \
  Piece *internal_##name##_3(Piece *callee, void *backpack) {                  \
    struct _Backpack_##name##_2 *pack = backpack;                              \
    return apply(callee, piece_create_##type(pack->i1 op pack->i2));           \
  }

INTERNAL_GENERATE_2(add, +, int)
INTERNAL_GENERATE_2(sub, -, int)
INTERNAL_GENERATE_2(mul, *, int)
INTERNAL_GENERATE_2(div, /, int)
INTERNAL_GENERATE_2(lt, <, bool)
INTERNAL_GENERATE_2(eq, ==, bool)

Piece *internal_end(Piece *callee, void *backpack) { return NULL; }

Piece *internal_if_2(Piece *, void *);

Piece *internal_if(Piece *callee, void *backpack) {
  printf("if\n");
  return piece_create(internal_if_2, callee->backpack);
}

typedef struct {
  Piece *left;
  bool cond;
} BackpackIf2;

Piece *internal_if_3(Piece *, void *);

Piece *internal_if_2(Piece *callee, void *backpack) {
  printf("if2\n");
  bool *p_b = backpack;
  BackpackIf2 *pack = malloc(sizeof(BackpackIf2));
  pack->left = callee;
  pack->cond = *p_b;
  return piece_create(internal_if_3, pack);
}

Piece *internal_if_3(Piece *callee, void *backpack) {
  BackpackIf2 *pack = backpack;
  printf("if3 cond: %d\n", pack->cond);
  return pack->cond ? pack->left : callee;
}

// Part 4, driver
Source *main_create_source(char *file_name) {
  FILE *source_file = fopen(file_name, "r");
  if (!source_file) {
    fprintf(stderr, "cannot open file \"%s\"\n", file_name);
    exit(ERROR_CANNOT_OPEN_FILE);
  }

  char *source = NULL;
  int length = 0;
  while (true) {
    char *line = NULL;
    int line_len = 0;
    line_len = (int)getline(&line, (size_t *)&line_len, source_file);
    if (line_len < 0) {
      break;
    }
    if (source == NULL) {
      source = line;
    } else {
      source = realloc(source, sizeof(char) * (length + line_len + 1));
      strcat(source, line);
    }
    length += line_len;
  }
  fclose(source_file);
  return source_create(source, length, file_name);
}

#define MAIN_REGISTER_INTERNAL(table, name, func, backpack)                    \
  table_register(table, name, sizeof(name) - 1, piece_create(func, backpack))

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "please specify source file by command line argument\n");
    exit(ERROR_NO_ARGV);
  }

  Table *table = table_create(NULL);
  MAIN_REGISTER_INTERNAL(table, "put", internal_put, NULL);
  MAIN_REGISTER_INTERNAL(table, "+", internal_add, NULL);
  MAIN_REGISTER_INTERNAL(table, "-", internal_sub, NULL);
  MAIN_REGISTER_INTERNAL(table, "<", internal_lt, NULL);
  MAIN_REGISTER_INTERNAL(table, "if", internal_if, NULL);

  Piece *p = parse_sentence(main_create_source(argv[1]), table);
  apply(p, piece_create(internal_end, NULL));
}
