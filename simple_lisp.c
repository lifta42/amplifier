// A simple parser and runtime for minimal LISP.
// Created: 12:35, 2017.10.30 by liftA42.
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 32
#define ENV_INIT_SIZE 8
#define LIST_MAX_SIZE 32

#define DISPLAY_LIST_MAX_ITEM 3

#define PRE_PARSE_ERROR 1
#define PARSE_ERROR 2
#define RUNTIME_ERROR 3

// Part 1: data structure
// notice: store primitive types in place, refer `struct X`  and strings with
// pointer
enum ElementType {
  TYPE_INT,
  TYPE_BOOL,
  TYPE_NAME,
  TYPE_LIST,
  TYPE_BUILTIN,
  TYPE_LAMBDA,
  TYPE_NULL // nil
};

struct ElementList;
struct Env;

struct Lambda {
  char *args[MAX_NAME_LEN];
  int arg_count;
  struct Env *parent;
  struct ElementList *body;
};

typedef struct Element *(*BuiltinFunc)(struct Element **, int, struct Env *);

struct Builtin {
  BuiltinFunc func;
  struct Env *parent;
};

struct Element {
  enum ElementType type;
  union {
    int int_value;
    int bool_value;
    char *name_value;
    struct ElementList *list_value;
    struct Builtin *builtin_value;
    struct Lambda *lambda_value;
  } value;
  // line number and col position help a lot... maybe next level
};

struct ElementList {
  struct Element **elements;
  int length;
};

struct Element *create_element_null() {
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
}

// notice: this function do NOT create a `ElementList`
struct Element *create_element_list(int length) {
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_LIST;
  ele->value.list_value = malloc(sizeof(struct ElementList));
  ele->value.list_value->length = length;
  ele->value.list_value->elements = malloc(sizeof(struct Element *) * length);
  return ele;
}

struct EnvPair {
  char *name;
  struct Element *value;
};

struct Env {
  struct EnvPair **values;
  int size;
  int length;
  struct Env *parent;
};

struct Env *create_env(struct Env *parent) {
  struct Env *env = malloc(sizeof(struct Env));
  env->parent = parent;
  env->values = malloc(sizeof(struct EnvPair *) * ENV_INIT_SIZE);
  env->size = ENV_INIT_SIZE;
  env->length = 0;
  return env;
}

struct Element *resolve(char *name, struct Env *env) {
  if (env == NULL) {
    fprintf(stderr, "there is no var called %s\n", name);
    exit(RUNTIME_ERROR);
  }
  for (int i = 0; i < env->length; i++) {
    if (strcmp(name, env->values[i]->name) == 0) {
      return env->values[i]->value;
    }
  }
  return resolve(name, env->parent);
}

void register_(struct Env *env, char *name, struct Element *value) {
  for (int i = 0; i < env->length; i++) {
    if (strcmp(name, env->values[i]->name) == 0) {
      env->values[i]->value = value;
      return;
    }
  }
  if (env->length == env->size) {
    env->values =
        realloc(env->values, sizeof(struct EnvPair *) * env->size * 2);
    env->size *= 2;
  }
  env->values[env->length] = malloc(sizeof(struct EnvPair));
  env->values[env->length]->name = name;
  env->values[env->length]->value = value;
  env->length++;
}

// Part 2: interpret loop
// notice: only modify `Env`s, only create others and refer to existing items
struct Element *eval(struct Element *ele, struct Env *env);
struct Element *apply(struct Element *ele, struct Element **args,
                      int arg_count);

// `lambda`, `cond` and `define` accepts un-evaluated elements as arguments, so
// they are not able to be built-ins, only compiler plugins (macros).
struct Element *eval_lambda(struct Element *lambda, struct Env *parent) {
  assert(lambda->value.list_value->length >= 2);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_LAMBDA;
  ele->value.lambda_value = malloc(sizeof(struct Lambda));
  ele->value.lambda_value->parent = parent;
  struct Element *arg_list = lambda->value.list_value->elements[1];
  assert(arg_list->type == TYPE_LIST);
  ele->value.lambda_value->arg_count = arg_list->value.list_value->length;
  for (int i = 0; i < ele->value.lambda_value->arg_count; i++) {
    assert(arg_list->value.list_value->elements[i]->type == TYPE_NAME);
    ele->value.lambda_value->args[i] =
        arg_list->value.list_value->elements[i]->value.name_value;
  }
  ele->value.lambda_value->body = malloc(sizeof(struct ElementList));
  int body_length = lambda->value.list_value->length - 2;
  ele->value.lambda_value->body->length = body_length;
  ele->value.lambda_value->body->elements =
      malloc(sizeof(struct Element *) * body_length);
  for (int i = 0; i < body_length; i++) {
    ele->value.lambda_value->body->elements[i] =
        lambda->value.list_value->elements[i + 2];
  }
  return ele;
}

struct Element *eval_quote_impl(struct Element *, struct Element *);

struct Element *eval_quote(struct Element *quoted, struct Env *parent) {
  assert(quoted->value.list_value->length == 3);
  struct Element *join = eval(quoted->value.list_value->elements[1], parent),
                 *raw = quoted->value.list_value->elements[2];
  assert(join->type == TYPE_LAMBDA || join->type == TYPE_BUILTIN);
  return eval_quote_impl(raw, join);
}

struct Element *eval_quote_impl(struct Element *raw, struct Element *join) {
  if (raw->type != TYPE_LIST) {
    return raw;
  } else {
    int child_count = raw->value.list_value->length;
    // maybe `children` is a better name
    struct Element **child_list =
        malloc(sizeof(struct Element *) * child_count);
    for (int i = 0; i < child_count; i++) {
      child_list[i] = eval_quote_impl(raw->value.list_value->elements[i], join);
    }
    return apply(join, child_list, child_count);
  }
}

struct Element *eval_cond(struct Element *cond, struct Env *parent) {
  assert(cond->value.list_value->length >= 2);
  for (int i = 1; i < cond->value.list_value->length; i++) {
    struct Element *branch = cond->value.list_value->elements[i];
    assert(branch->type == TYPE_LIST);
    assert(branch->value.list_value->length == 2);
    struct Element *branch_cond = branch->value.list_value->elements[0];
    if (branch_cond->type == TYPE_NAME &&
        strcmp(branch_cond->value.name_value, "else") == 0) {
      return eval(branch->value.list_value->elements[1], parent);
    }
    struct Element *condition = eval(branch_cond, parent);
    assert(condition->type == TYPE_BOOL);
    if (condition->value.bool_value) {
      return eval(branch->value.list_value->elements[1], parent);
    }
  }
  return create_element_null();
}

struct Element *eval_define(struct Element *def, struct Env *parent) {
  assert(def->value.list_value->length == 3);
  struct Element *name = def->value.list_value->elements[1];
  // dynamic name is not allowed
  assert(name->type == TYPE_NAME);
  register_(parent, name->value.name_value,
            eval(def->value.list_value->elements[2], parent));

  return create_element_null();
}

struct Element *eval(struct Element *ele, struct Env *env) {
  switch (ele->type) {
  case TYPE_INT:
  case TYPE_BOOL:
  case TYPE_BUILTIN:
  case TYPE_LAMBDA:
  case TYPE_NULL:
    return ele;
  case TYPE_NAME:
    return resolve(ele->value.name_value, env);
  case TYPE_LIST: {
    struct Element *callable = ele->value.list_value->elements[0];
    if (callable->type == TYPE_NAME) {
      if (strcmp(callable->value.name_value, "lambda") == 0) {
        return eval_lambda(ele, env);
      } else if (strcmp(callable->value.name_value, "cond") == 0) {
        return eval_cond(ele, env);
      } else if (strcmp(callable->value.name_value, "define") == 0) {
        return eval_define(ele, env);
      } else if (strcmp(callable->value.name_value, "quote") == 0) {
        return eval_quote(ele, env);
      } else {
        callable = resolve(callable->value.name_value, env);
      }
    }
    // IIFE style lambda
    if (callable->type == TYPE_LIST) {
      callable = eval(callable, env);
    }
    int arg_count = ele->value.list_value->length - 1;
    // strict order
    struct Element **args = malloc(sizeof(struct Element *) * arg_count);
    for (int i = 0; i < arg_count; i++) {
      args[i] = eval(ele->value.list_value->elements[i + 1], env);
    }
    return apply(callable, args, arg_count);
  }
  }
}

struct Element *apply(struct Element *ele, struct Element **args,
                      int arg_count) {
  if (ele->type != TYPE_LAMBDA && ele->type != TYPE_BUILTIN) {
    fprintf(stderr, "apply an un-callable element\n");
    exit(RUNTIME_ERROR);
  }
  if (ele->type == TYPE_BUILTIN) {
    // built-in does not need its own env, just use its parent's one if exists
    return ele->value.builtin_value->func(args, arg_count,
                                          ele->value.builtin_value->parent);
  } else {
    assert(ele->value.lambda_value->arg_count == arg_count);
    // lambda has a static var scope, nothing to do with `parent` argument,
    // which comes from calling scope
    struct Env *env = create_env(ele->value.lambda_value->parent);
    for (int i = 0; i < ele->value.lambda_value->arg_count; i++) {
      register_(env, ele->value.lambda_value->args[i], args[i]);
    }
    struct Element *result = malloc(sizeof(struct Element));
    result->type = TYPE_NULL; // fallback for (lambda (...) )
    for (int i = 0; i < ele->value.lambda_value->body->length; i++) {
      result = eval(ele->value.lambda_value->body->elements[i], env);
    }
    return result;
  }
}

// Part 3: parser
// notice: create everything here
struct Element *parse(char *source, int *pos);

void parse_ignore_space(char *source, int *pos) {
  while (isspace(source[*pos])) {
    (*pos)++;
  }
}

struct Element *parse_comment(char *source, int *pos) {
  while (source[*pos] != '\n') {
    (*pos)++;
  }
  (*pos)++;
  while (isspace(source[*pos])) {
    (*pos)++;
  }
  if (source[*pos] == ')') {
    return NULL;
  }
  return parse(source, pos);
}

struct Element *parse_number(char *source, int *pos) {
  // only int actually
  int n = 0;
  while (isdigit(source[*pos])) {
    n = n * 10 + (source[*pos] - '0');
    (*pos)++;
  }
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_INT;
  ele->value.int_value = n;
  return ele;
}

struct Element *parse_string(char *source, int *pos) {
  // (car (quote list 'foo')) ==> (car (quote list (102 111 111))) ==>
  // (car (cons 102 (cons 111 (cons 111 nil)))) ==> 102
  // it must appear inside of `(quote some-join <here>)`, or calling
  // un-callable var (`102` above) exception would be raised
  // use single quotation mark so that `"` can appear without backslash
  // notice: only take effect when following a space, so `fac'` and
  // `six-o'clock` will be normal names
  (*pos)++;
  struct Element **char_seq = malloc(sizeof(struct Element *) * LIST_MAX_SIZE);
  int char_count = 0;
  while (source[*pos] != '\'') {
    char c = source[*pos];
    if (c == '\\') {
      (*pos)++;
      switch (source[*pos]) {
      case 'n':
        c = '\n';
        break;
      case 't':
        c = '\t';
        break;
      case '\'':
        c = '\'';
        break;
      case '\\':
        c = '\\';
        break;
      default:
        fprintf(stderr, "escape sequence \"\\%c\" is not recognizable\n",
                source[*pos]);
        exit(PARSE_ERROR);
      }
    }
    if (char_count == LIST_MAX_SIZE - 1) {
      // nothing else uses stack, so neither does this even stack is usable
      // same to `free()`
      char *literal = malloc(sizeof(char) * LIST_MAX_SIZE);
      for (int i = 0; i < LIST_MAX_SIZE - 1; i++) {
        literal[i] = (char)char_seq[i]->value.int_value;
      }
      literal[LIST_MAX_SIZE - 1] = '\0';
      fprintf(stderr,
              "string literal \"%s...\" exceed list length limitation\n",
              literal);
      exit(PARSE_ERROR);
    }
    struct Element *ele = malloc(sizeof(struct Element));
    ele->type = TYPE_INT;
    ele->value.int_value = (int)c;
    char_seq[char_count] = ele;
    char_count++;
    (*pos)++;
  }
  (*pos)++; // for ending quotes
  struct Element *ele = create_element_list(char_count);
  for (int i = 0; i < char_count; i++) {
    ele->value.list_value->elements[i] = char_seq[i];
  }
  return ele;
}

struct Element *parse_name(char *source, int *pos) {
  // nil is also here
  char *name = malloc(sizeof(char) * MAX_NAME_LEN);
  int name_len = 0;
  while (!isspace(source[*pos]) && source[*pos] != '(' && source[*pos] != ')') {
    if (name_len == MAX_NAME_LEN - 1) {
      name[name_len] = '\0';
      fprintf(stderr, "name \"%s...\" exceed name length limitation\n", name);
      exit(PARSE_ERROR);
    }
    name[name_len] = source[*pos];
    name_len++;
    (*pos)++;
  }
  name[name_len] = '\0';
  // alternative way is to define a built-in, but I do not like `(nil)`,
  // or to use something like `(define nil ((lambda () (define foo))))`
  // which I do not like either
  if (strcmp(name, "nil") == 0) {
    struct Element *ele = malloc(sizeof(struct Element));
    ele->type = TYPE_NULL;
    return ele;
  } else {
    struct Element *ele = malloc(sizeof(struct Element));
    ele->type = TYPE_NAME;
    ele->value.name_value = name;
    return ele;
  }
}

struct Element *parse_list(char *source, int *pos) {
  (*pos)++;
  int child_count = 0;
  struct Element *child_list[LIST_MAX_SIZE];

  parse_ignore_space(source, pos);
  while (source[*pos] != ')') {
    child_list[child_count] = parse(source, pos);
    // check postfix comment
    if (child_list[child_count] != NULL) {
      child_count++;
    }
    parse_ignore_space(source, pos);
  }
  (*pos)++; // for ')'
  struct Element *ele = create_element_list(child_count);
  for (int i = 0; i < child_count; i++) {
    ele->value.list_value->elements[i] = child_list[i];
  }
  return ele;
}

// this function will return NULL (not element with type TYPE_NULL) only in two
// cases:
// 1. reaching '\0' before anything else
// 2. reaching ')' before anything else after parsing a comment line
struct Element *parse(char *source, int *pos) {
  parse_ignore_space(source, pos);

  if (source[*pos] == '\0') {
    return NULL;
  }

  if (source[*pos] == ';') {
    return parse_comment(source, pos);
  }

  if (source[*pos] == ')') {
    fprintf(stderr, "unexpected \")\" appeared\n");
    exit(PARSE_ERROR);
  }

  if (source[*pos] != '(') {
    if (isdigit(source[*pos])) {
      return parse_number(source, pos);
    } else if (source[*pos] == '\'') {
      return parse_string(source, pos);
    } else {
      return parse_name(source, pos);
    }
  } else {
    return parse_list(source, pos);
  }
}

// Part 4: built-in
struct Element *builtin_add(struct Element **args, int arg_count,
                            struct Env *parent) {
  assert(arg_count == 2);
  assert(args[0]->type == TYPE_INT);
  assert(args[1]->type == TYPE_INT);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_INT;
  ele->value.int_value = args[0]->value.int_value + args[1]->value.int_value;
  return ele;
}

struct Element *builtin_display(struct Element **args, int arg_count,
                                struct Env *parent) {
  assert(arg_count == 1);
  switch (args[0]->type) {
  case TYPE_INT:
    printf("%d", args[0]->value.int_value);
    break;
  case TYPE_BOOL:
    printf(args[0]->value.bool_value ? "true" : "false");
    break;
  case TYPE_NULL:
    printf("nil");
    break;
  case TYPE_NAME:
    printf("<name: %s>", args[0]->value.name_value);
    break;
  case TYPE_BUILTIN:
    printf("<built-in %p>", args[0]->value.builtin_value);
    break;
  case TYPE_LAMBDA:
    printf("<lambda (");
    char *prefix = "";
    for (int i = 0; i < args[0]->value.lambda_value->arg_count; i++) {
      printf("%s%s", prefix, args[0]->value.lambda_value->args[i]);
      prefix = " ";
    }
    printf(")>");
    break;
  case TYPE_LIST:
    // it is impossible that a internal list becomes evaled argument
    assert(0);
    break;
  }
  return create_element_null();
}

struct Element *builtin_eq(struct Element **args, int arg_count,
                           struct Env *parent) {
  assert(arg_count == 2);
  assert(args[0]->type == TYPE_INT);
  assert(args[1]->type == TYPE_INT);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BOOL;
  ele->value.bool_value = args[0]->value.int_value == args[1]->value.int_value;
  return ele;
}

struct Element *builtin_gt(struct Element **args, int arg_count,
                           struct Env *parent) {
  assert(arg_count == 2);
  assert(args[0]->type == TYPE_INT);
  assert(args[1]->type == TYPE_INT);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BOOL;
  ele->value.bool_value = args[0]->value.int_value > args[1]->value.int_value;
  return ele;
}

struct Element *builtin_mul(struct Element **args, int arg_count,
                            struct Env *parent) {
  assert(arg_count == 2);
  assert(args[0]->type == TYPE_INT);
  assert(args[1]->type == TYPE_INT);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_INT;
  ele->value.int_value = args[0]->value.int_value * args[1]->value.int_value;
  return ele;
}

struct Element *builtin_sub(struct Element **args, int arg_count,
                            struct Env *parent) {
  assert(arg_count == 2);
  assert(args[0]->type == TYPE_INT);
  assert(args[1]->type == TYPE_INT);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_INT;
  ele->value.int_value = args[0]->value.int_value - args[1]->value.int_value;
  return ele;
}

// this is inevitable because there's no `string` or `char` type
struct Element *builtin_newline(struct Element **args, int arg_count,
                                struct Env *parent) {
  assert(arg_count == 0);
  printf("\n");
  return create_element_null();
}

struct Element *builtin_is_nil(struct Element **args, int arg_count,
                               struct Env *parent) {
  assert(arg_count == 1);
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BOOL;
  ele->value.bool_value = args[0]->type == TYPE_NULL;
  return ele;
}

struct Element *builtin_exit(struct Element **args, int arg_count,
                             struct Env *parent) {
  if (arg_count == 0) {
    exit(0);
  }
  assert(args[0]->type == TYPE_INT);
  exit(args[0]->value.int_value);
  // trivial
  return create_element_null();
}

#define FOLDR_CALLBACK_REGISTERED_NAME "reserved_foldr-callback"
#define FOLDR_INIT_REGISTERED_NAME "reserved_foldr-init"
struct Element *builtin_foldr_impl(struct Element **, int, struct Env *);
// (foldr callback init) ==>
// <env <built-in (args...) ...> >
struct Element *builtin_foldr(struct Element **args, int arg_count,
                              struct Env *parent) {
  assert(arg_count == 2);
  struct Element *callback = args[0], *init = args[1];
  assert(callback->type == TYPE_LAMBDA || callback->type == TYPE_BUILTIN);

  struct Env *env = create_env(parent);
  register_(env, FOLDR_CALLBACK_REGISTERED_NAME, callback);
  register_(env, FOLDR_INIT_REGISTERED_NAME, init);

  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BUILTIN;
  ele->value.builtin_value = malloc(sizeof(struct Builtin));
  ele->value.builtin_value->func = builtin_foldr_impl;
  ele->value.builtin_value->parent = env;
  return ele;
}

struct Element *builtin_foldr_impl(struct Element **args, int arg_count,
                                   struct Env *parent) {
  struct Element *callback = resolve(FOLDR_CALLBACK_REGISTERED_NAME, parent),
                 *init = resolve(FOLDR_INIT_REGISTERED_NAME, parent);
  struct Element *product = init;
  for (int i = arg_count - 1; i >= 0; i--) {
    struct Element *apply_args[2] = {product, args[i]};
    product = apply(callback, apply_args, 2);
  }
  return product;
}

#define PIPE_FUNC_LIST_REGISTERED_NAME "reserved_pipe-func-list"
struct Element *builtin_pipe_impl(struct Element **, int, struct Env *);
// (pipe f g) with f :: (lambda (<args: length=x>) ...), g :: (lambda (x) ...)
// ==> (lambda (<args: length=x>) (g (f args...)))
struct Element *builtin_pipe(struct Element **args, int arg_count,
                             struct Env *parent) {
  assert(arg_count >= 2);
  struct Element *func_list = malloc(sizeof(struct Element *));
  func_list->type = TYPE_LIST;
  func_list->value.list_value = malloc(sizeof(struct ElementList *));
  func_list->value.list_value->length = arg_count;
  func_list->value.list_value->elements =
      malloc(sizeof(struct Element *) * arg_count);
  for (int i = 0; i < arg_count; i++) {
    assert(args[i]->type == TYPE_LAMBDA || args[i]->type == TYPE_BUILTIN);
    func_list->value.list_value->elements[i] = args[i];
  }

  struct Env *env = create_env(parent);
  register_(env, PIPE_FUNC_LIST_REGISTERED_NAME, func_list);

  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BUILTIN;
  ele->value.builtin_value = malloc(sizeof(struct Builtin));
  ele->value.builtin_value->func = builtin_pipe_impl;
  ele->value.builtin_value->parent = env;

  return ele;
}

struct Element *builtin_pipe_impl(struct Element **args, int arg_count,
                                  struct Env *parent) {
  struct Element *func_list = resolve(PIPE_FUNC_LIST_REGISTERED_NAME, parent);
  struct Element **current_args = args;
  struct Element *current_result = NULL; // to prevent rvalue ref
  int current_arg_count = arg_count;
  for (int i = 0; i < func_list->value.list_value->length; i++) {
    struct Element *func = func_list->value.list_value->elements[i];
    current_result = apply(func, current_args, current_arg_count);
    current_args = &current_result;
    current_arg_count = 1;
  }
  return current_args[0];
}

struct Element *builtin_display_char(struct Element **args, int arg_count,
                                     struct Env *parent) {
  assert(arg_count == 1);
  assert(args[0]->type == TYPE_INT);
  int code = args[0]->value.int_value;
  if (code > 255 || !isprint(code)) {
    // this usually happens in the middle of string printing, so insert a line
    // break to prevent corrupting
    // this case does not happen on macOS, since it flushes stdout per line
    fprintf(stderr, "\nattempt to display out-of-range char: %d\n", code);
    exit(RUNTIME_ERROR);
  } else {
    printf("%c", (char)code);
  }
  return create_element_null();
}

void register_builtin(struct Env *env, char *name, BuiltinFunc builtin) {
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BUILTIN;
  ele->value.builtin_value = malloc(sizeof(struct Builtin));
  ele->value.builtin_value->func = builtin;
  ele->value.builtin_value->parent = NULL; // so sad it does not have parent
  register_(env, name, ele);
}

void register_argv(struct Env *env, int argc, char *argv[]) {
  struct Element *argv_list = create_element_list(argc);
  for (int i = 0; i < argc; i++) {
    int argv_len = strlen(argv[i]);
    if (argv_len > LIST_MAX_SIZE) {
      fprintf(stderr, "warning: argv \"%s\" is too long and will be cropped\n",
              argv[i]);
      argv_len = LIST_MAX_SIZE;
    }
    struct Element *ele = create_element_list(argv_len);
    for (int j = 0; j < argv_len; j++) {
      ele->value.list_value->elements[j] = malloc(sizeof(struct Element));
      ele->value.list_value->elements[j]->type = TYPE_INT;
      ele->value.list_value->elements[j]->value.int_value = (int)argv[i][j];
    }
    argv_list->value.list_value->elements[i] = ele;
  }
  register_(env, "argv", argv_list);
}

// Part 5: driver
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please specify entry source file as command line arguement.\n");
    return 0;
  }
  struct Env *env = create_env(NULL);
  register_argv(env, argc, argv);
  register_builtin(env, "+", builtin_add);
  register_builtin(env, "-", builtin_sub);
  register_builtin(env, "*", builtin_mul);
  register_builtin(env, "display", builtin_display);
  register_builtin(env, "newline", builtin_newline);
  register_builtin(env, "=", builtin_eq);
  register_builtin(env, ">", builtin_gt);
  register_builtin(env, "nil?", builtin_is_nil);
  register_builtin(env, "exit", builtin_exit);
  register_builtin(env, "foldr", builtin_foldr);
  register_builtin(env, "pipe", builtin_pipe);
  register_builtin(env, "display-char", builtin_display_char);

  char *source = NULL;
  int len = 0;
  // read whole file into `source`
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "can not open file \"%s\"\n", argv[1]);
    exit(PRE_PARSE_ERROR);
  }
  while (1) {
    char *line = NULL;
    int line_len = 0;
    line_len = (int)getline(&line, (size_t *)&line_len, file);
    if (line_len < 0) {
      break;
    }
    if (source == NULL) {
      source = line;
    } else {
      source = realloc(source, sizeof(char) * (len + line_len + 1));
      strcat(source, line);
    }
    len += line_len;
  }
  fclose(file);

  int pos = 0;
  while (pos != len) {
    struct Element *root = parse(source, &pos);
    if (root != NULL) {
      eval(root, env);
    }
  }
}
