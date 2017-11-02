// A simple parser and runtime for minimal LISP.
// 12:35, 2017.10.30 by liftA42.
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 32
#define ENV_INIT_SIZE 8
#define LIST_MAX_SIZE 32
#define SRC_INIT_LEN 1024

#define DISPLAY_LIST_MAX_ITEM 3

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
  TYPE_NULL
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
    exit(1);
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

struct Element *eval(struct Element *ele, struct Env *env);

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
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
}

struct Element *eval_define(struct Element *def, struct Env *parent) {
  assert(def->value.list_value->length == 3);
  struct Element *name = def->value.list_value->elements[1];
  // dynamic name is not allowed
  assert(name->type == TYPE_NAME);
  register_(parent, name->value.name_value,
            eval(def->value.list_value->elements[2], parent));

  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
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
    exit(1);
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
struct Element *parse(char *source, int *pos) {
  while (isspace(source[*pos])) {
    (*pos)++;
  }

  if (source[*pos] == '\0') {
    struct Element *element = malloc(sizeof(struct Element));
    element->type = TYPE_NULL;
    return element;
  }

  if (source[*pos] == ';') {
    while (source[*pos] != '\n') {
      (*pos)++;
    }
    (*pos)++;
    return parse(source, pos);
  }

  if (source[*pos] != '(') {
    if (isdigit(source[*pos])) {
      int n = 0;
      while (isdigit(source[*pos])) {
        n = n * 10 + (source[*pos] - '0');
        (*pos)++;
      }
      struct Element *ele = malloc(sizeof(struct Element));
      ele->type = TYPE_INT;
      ele->value.int_value = n;
      return ele;
    } else {
      char *name = malloc(sizeof(char) * MAX_NAME_LEN);
      int name_len = 0;
      while (!isspace(source[*pos]) && source[*pos] != ')') {
        if (name_len == MAX_NAME_LEN - 1) {
          source[MAX_NAME_LEN - 1] = '\0';
          fprintf(stderr, "name %s... exceed name length limitation\n", source);
          exit(1);
        }
        name[name_len] = source[*pos];
        name_len++;
        (*pos)++;
      }
      name[name_len] = '\0';
      // alternative way is to define a built-in, but I do not like `(nil)`
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
  } else {
    (*pos)++;
    struct Element *ele = malloc(sizeof(struct Element));
    ele->type = TYPE_LIST;
    ele->value.list_value = malloc(sizeof(struct ElementList));
    int child_count = 0;
    struct Element *child_list[LIST_MAX_SIZE];

    while (isspace(source[*pos])) {
      (*pos)++;
    }
    while (source[*pos] != ')') {
      child_list[child_count] = parse(source, pos);
      child_count++;
      while (isspace(source[*pos])) {
        (*pos)++;
      }
    }
    (*pos)++; // for ')'
    ele->value.list_value->length = child_count;
    ele->value.list_value->elements =
        malloc(sizeof(struct Element *) * child_count);
    for (int i = 0; i < child_count; i++) {
      ele->value.list_value->elements[i] = child_list[i];
    }
    return ele;
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
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
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
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
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
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_NULL;
  return ele;
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
  register_(env, "_func_list", func_list);

  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BUILTIN;
  ele->value.builtin_value = malloc(sizeof(struct Builtin));
  ele->value.builtin_value->func = builtin_pipe_impl;
  ele->value.builtin_value->parent = env;

  return ele;
}

struct Element *builtin_pipe_impl(struct Element **args, int arg_count,
                                  struct Env *parent) {
  struct Element *func_list = resolve("_func_list", parent);
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

void register_builtin(struct Env *env, char *name, BuiltinFunc builtin) {
  struct Element *ele = malloc(sizeof(struct Element));
  ele->type = TYPE_BUILTIN;
  ele->value.builtin_value = malloc(sizeof(struct Builtin));
  ele->value.builtin_value->func = builtin;
  ele->value.builtin_value->parent = NULL; // so sad it does not have parent
  register_(env, name, ele);
}

// Part 5: driver
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please specify entry source file as command line arguement.\n");
    return 0;
  }
  struct Env *env = create_env(NULL);
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

  char *source = NULL;
  int len = 0;
  // read whole file into `source`
  FILE *file = fopen(argv[1], "r");
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
    eval(parse(source, &pos), env);
  }
}
