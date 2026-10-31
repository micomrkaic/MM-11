/*
 * This file is part of MM-11.
 *
 * MM-11 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MM-11 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MM-11 If not, see <https://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "words.h"
#include "ops.h"
#include "stack.h"
#include "tokenizer.h"
#include "util_func.h"
#include "help_functions.h"

#define WORD_FILE "words.txt"

UserWord *words = NULL;
int word_count = 0;
static int word_cap = 0;
static int exec_level = 0;   /* >0 while a word runs: word table must not change */

static const char *control_words[] = {
  "if", "else", "then", "begin", "until", "while", "repeat",
  "do", "loop", "+loop", "i", "j", NULL
};

bool is_control_word(const char *t) {
  for (int i = 0; control_words[i]; i++)
    if (strcmp(t, control_words[i]) == 0) return true;
  return false;
}

int find_word(const char *name) {
  for (int i = 0; i < word_count; i++)
    if (strcmp(words[i].name, name) == 0) return i;
  return -1;
}

static void free_code(Instr *code, int n) {
  if (!code) return;
  for (int i = 0; i < n; i++)
    if (code[i].k == I_CALL) free(code[i].u.name);
  free(code);
}

static void free_word(UserWord *w) {
  free(w->body);
  free_code(w->code, w->ncode);
  w->body = NULL;
  w->code = NULL;
  w->ncode = 0;
}

static int guard_mutation(const char *what) {
  if (exec_level > 0) return rpn_err("%s cannot be used while a word is running", what);
  return RPN_OK;
}

/* ======================= compiler ======================= */

typedef enum { C_IF, C_ELSE, C_BEGIN, C_WHILE, C_DO } CtlKind;
typedef struct { CtlKind k; int pc; } Ctl;

static const char *ctl_name(CtlKind k) {
  switch (k) {
  case C_IF: return "if";
  case C_ELSE: return "else";
  case C_BEGIN: return "begin";
  case C_WHILE: return "while";
  case C_DO: return "do";
  }
  return "?";
}

static int compile(const char *self, char **toks, int n, Instr **out, int *nout) {
  Instr *code = calloc((size_t)n + 1, sizeof *code);
  if (!code) return rpn_err("Out of memory");
  Ctl ctl[64];
  int nctl = 0, ndo = 0, pc = 0, rc = RPN_OK;

#define FAIL(...) do { rc = rpn_err(__VA_ARGS__); goto fail; } while (0)
#define NEED(kind, tok)                                                         \
  do { if (nctl == 0 || ctl[nctl - 1].k != (kind))                              \
         FAIL("%s: '%s' without matching '%s'", self, tok, ctl_name(kind)); } while (0)
#define PUSHCTL(kind)                                                           \
  do { if (nctl >= 64) FAIL("%s: control structures nested too deeply", self); \
       ctl[nctl].k = (kind); ctl[nctl].pc = pc; nctl++; } while (0)

  for (int i = 0; i < n; i++) {
    const char *t = toks[i];
    double v;
    NumParse np = parse_number(t, &v);
    if (np == NUM_RANGE) FAIL("%s: number out of range: %s", self, t);
    if (np == NUM_OK) { code[pc].k = I_NUM; code[pc].u.num = v; pc++; continue; }

    if (!strcmp(t, "if")) {
      PUSHCTL(C_IF);
      code[pc].k = I_BRANCH0; code[pc].u.target = -1; pc++;
    } else if (!strcmp(t, "else")) {
      NEED(C_IF, t);
      code[pc].k = I_BRANCH; code[pc].u.target = -1;
      code[ctl[nctl - 1].pc].u.target = pc + 1;
      ctl[nctl - 1].k = C_ELSE; ctl[nctl - 1].pc = pc;
      pc++;
    } else if (!strcmp(t, "then")) {
      if (nctl == 0 || (ctl[nctl - 1].k != C_IF && ctl[nctl - 1].k != C_ELSE))
        FAIL("%s: 'then' without matching 'if'", self);
      code[ctl[--nctl].pc].u.target = pc;
    } else if (!strcmp(t, "begin")) {
      PUSHCTL(C_BEGIN);
    } else if (!strcmp(t, "until")) {
      NEED(C_BEGIN, t);
      code[pc].k = I_BRANCH0; code[pc].u.target = ctl[--nctl].pc; pc++;
    } else if (!strcmp(t, "while")) {
      NEED(C_BEGIN, t);
      PUSHCTL(C_WHILE);
      code[pc].k = I_BRANCH0; code[pc].u.target = -1; pc++;
    } else if (!strcmp(t, "repeat")) {
      NEED(C_WHILE, t);
      int wpc = ctl[--nctl].pc;
      int bpc = ctl[--nctl].pc;            /* the BEGIN under WHILE */
      code[pc].k = I_BRANCH; code[pc].u.target = bpc; pc++;
      code[wpc].u.target = pc;
    } else if (!strcmp(t, "do")) {
      if (ndo >= MAX_LOOP_NEST) FAIL("%s: do loops nested deeper than %d", self, MAX_LOOP_NEST);
      PUSHCTL(C_DO);
      code[pc].k = I_DO; code[pc].u.target = -1; pc++;
      ndo++;
    } else if (!strcmp(t, "loop") || !strcmp(t, "+loop")) {
      NEED(C_DO, t);
      int dpc = ctl[--nctl].pc;
      code[pc].k = (t[0] == '+') ? I_PLOOP : I_LOOP;
      code[pc].u.target = dpc + 1; pc++;
      code[dpc].u.target = pc;
      ndo--;
    } else if (!strcmp(t, "i")) {
      if (ndo < 1) FAIL("%s: 'i' used outside a do loop", self);
      code[pc++].k = I_I;
    } else if (!strcmp(t, "j")) {
      if (ndo < 2) FAIL("%s: 'j' needs two nested do loops", self);
      code[pc++].k = I_J;
    } else {
      int op = find_op(t);
      if (op >= 0) {
        if (!ops[op].fn) FAIL("%s: '%s' cannot be used inside a definition", self, t);
        code[pc].k = I_OP; code[pc].u.op = op; pc++;
      } else if (strcmp(t, self) == 0 || find_word(t) >= 0) {
        code[pc].k = I_CALL;
        code[pc].u.name = strdup(t);
        if (!code[pc].u.name) FAIL("Out of memory");
        pc++;
      } else {
        FAIL("%s: undefined word '%s'", self, t);
      }
    }
  }
  if (nctl > 0) FAIL("%s: unterminated '%s'", self, ctl_name(ctl[nctl - 1].k));
  *out = code;
  *nout = pc;
  return RPN_OK;

fail:
  free_code(code, pc);
  return rc;
#undef FAIL
#undef NEED
#undef PUSHCTL
}

static char *join_tokens(char **toks, int n) {
  size_t len = 1;
  for (int i = 0; i < n; i++) len += strlen(toks[i]) + 1;
  char *s = malloc(len), *p = s;
  if (!s) return NULL;
  *p = '\0';
  for (int i = 0; i < n; i++) {
    size_t l = strlen(toks[i]);
    if (i) *p++ = ' ';
    memcpy(p, toks[i], l);
    p += l;
  }
  *p = '\0';
  return s;
}

static int validate_name(const char *name) {
  size_t l = strlen(name);
  if (l == 0) return rpn_err("Missing word name");
  if (l >= MAX_WORD_NAME) return rpn_err("Word name longer than %d characters", MAX_WORD_NAME - 1);
  if (parse_number(name, NULL) != NUM_NO) return rpn_err("'%s' is a number, not a valid name", name);
  if (find_op(name) >= 0) return rpn_err("'%s' is a built-in command", name);
  if (is_control_word(name)) return rpn_err("'%s' is a control word", name);
  return RPN_OK;
}

static int grow(void) {
  if (word_count < word_cap) return RPN_OK;
  int nc = word_cap ? 2 * word_cap : 32;
  UserWord *nw = realloc(words, (size_t)nc * sizeof *nw);
  if (!nw) return rpn_err("Out of memory");
  words = nw;
  word_cap = nc;
  return RPN_OK;
}

int define_word(const char *name, char **toks, int ntok) {
  TRY(guard_mutation("Defining a word"));
  TRY(validate_name(name));
  Instr *code;
  int ncode;
  TRY(compile(name, toks, ntok, &code, &ncode));
  char *body = join_tokens(toks, ntok);
  if (!body) { free_code(code, ncode); return rpn_err("Out of memory"); }

  int idx = find_word(name);
  if (idx >= 0) {
    free_word(&words[idx]);
  } else {
    if (grow() != RPN_OK) { free(body); free_code(code, ncode); return RPN_ERR; }
    idx = word_count++;
    snprintf(words[idx].name, MAX_WORD_NAME, "%s", name);
  }
  words[idx].body = body;
  words[idx].code = code;
  words[idx].ncode = ncode;
  rpn_info("Defined %s: %s", name, body);
  return RPN_OK;
}

/* ======================= executor ======================= */

int exec_word(State *s, int idx, int depth) {
  if (depth >= MAX_RECURSION_DEPTH)
    return rpn_err("Recursion deeper than %d levels (in '%s')", MAX_RECURSION_DEPTH, words[idx].name);
  /* Safe to hold: the word table cannot change while exec_level > 0. */
  const UserWord *w = &words[idx];
  struct { double index, limit; } lp[MAX_LOOP_NEST];
  int nl = 0, rc = RPN_OK;
  Stack *st = &s->stack;

  exec_level++;
  for (int pc = 0; pc < w->ncode && rc == RPN_OK; ) {
    if (interrupted) { rc = rpn_err("Interrupted"); break; }
    const Instr *in = &w->code[pc];
    switch (in->k) {
    case I_NUM:
      rc = push(st, in->u.num); pc++;
      break;
    case I_OP:
      rc = call_op(s, in->u.op); pc++;
      break;
    case I_CALL: {
      int j = find_word(in->u.name);
      if (j < 0) rc = rpn_err("%s: undefined word '%s'", w->name, in->u.name);
      else rc = exec_word(s, j, depth + 1);
      pc++;
      break;
    }
    case I_BRANCH:
      pc = in->u.target;
      break;
    case I_BRANCH0:
      if (stack_depth(st) < 1) { rc = rpn_err("%s: conditional needs a flag on the stack", w->name); break; }
      pc = (pop(st) == 0.0) ? in->u.target : pc + 1;
      break;
    case I_DO: {
      if (stack_depth(st) < 2) { rc = rpn_err("%s: do needs limit and start", w->name); break; }
      double start = pop(st), limit = pop(st);
      if (start == limit) { pc = in->u.target; break; }
      lp[nl].index = start; lp[nl].limit = limit; nl++;
      pc++;
      break;
    }
    case I_LOOP:
      lp[nl - 1].index += 1.0;
      if (lp[nl - 1].index < lp[nl - 1].limit) pc = in->u.target;
      else { nl--; pc++; }
      break;
    case I_PLOOP: {
      if (stack_depth(st) < 1) { rc = rpn_err("%s: +loop needs an increment", w->name); break; }
      double inc = pop(st);
      lp[nl - 1].index += inc;
      bool again = (inc >= 0.0) ? lp[nl - 1].index < lp[nl - 1].limit
                                : lp[nl - 1].index >= lp[nl - 1].limit;
      if (again) pc = in->u.target;
      else { nl--; pc++; }
      break;
    }
    case I_I:
      rc = push(st, lp[nl - 1].index); pc++;
      break;
    case I_J:
      rc = push(st, lp[nl - 2].index); pc++;
      break;
    }
  }
  exec_level--;
  if (rc != RPN_OK && depth == 0) {
    fflush(stdout);
    fprintf(stderr, "  (in word '%s')\n", w->name);
  }
  return rc;
}

/* ======================= listing ======================= */

void list_words(void) {
  if (word_count == 0) { printf("  (no words defined)\n"); return; }
  for (int i = 0; i < word_count; i++)
    printf("  %2d  %-16s %s\n", i, words[i].name, words[i].body);
}

int see_word(const char *name) {
  int i = find_word(name);
  if (i >= 0) { printf(": %s %s ;\n", words[i].name, words[i].body); return RPN_OK; }
  int op = find_op(name);
  if (op >= 0) { describe_op(op); return RPN_OK; }
  return rpn_err("see: unknown word '%s'", name);
}

int delete_word(int idx) {
  TRY(guard_mutation("delword"));
  if (idx < 0 || idx >= word_count) return rpn_err("delword: no word number %d", idx);
  rpn_info("Deleted %s", words[idx].name);
  free_word(&words[idx]);
  memmove(&words[idx], &words[idx + 1], (size_t)(word_count - idx - 1) * sizeof *words);
  word_count--;
  return RPN_OK;
}

void free_words(void) {
  for (int i = 0; i < word_count; i++) free_word(&words[i]);
  free(words);
  words = NULL;
  word_count = word_cap = 0;
}

int clear_words(void) {
  TRY(guard_mutation("clearwords"));
  for (int i = 0; i < word_count; i++) free_word(&words[i]);
  word_count = 0;
  return RPN_OK;
}

/* ======================= persistence ======================= */

int save_words(void) {
  char path[1200], tmp[1210];
  TRY(config_path(WORD_FILE, path, sizeof path, true));
  snprintf(tmp, sizeof tmp, "%s.tmp", path);
  FILE *f = fopen(tmp, "w");
  if (!f) return rpn_err("Cannot write %s: %s", tmp, strerror(errno));
  fprintf(f, "# MM-11 user words: name followed by its body\n");
  for (int i = 0; i < word_count; i++)
    fprintf(f, "%s %s\n", words[i].name, words[i].body);
  if (fclose(f) != 0 || rename(tmp, path) != 0)
    return rpn_err("Cannot save %s: %s", path, strerror(errno));
  rpn_info("%d word%s saved to %s", word_count, word_count == 1 ? "" : "s", path);
  return RPN_OK;
}

/* Two passes: register every name first (bodies may refer to words defined
   later in the file), then compile. Words that fail to compile are dropped. */
int load_words(bool quiet_if_missing) {
  TRY(guard_mutation("loadwords"));
  char path[1200];
  TRY(config_path(WORD_FILE, path, sizeof path, false));
  FILE *f = fopen(path, "r");
  if (!f) {
    if (quiet_if_missing && errno == ENOENT) return RPN_OK;
    return rpn_err("Cannot read %s: %s", path, strerror(errno));
  }
  clear_words();

  char *line = NULL;
  size_t cap = 0;
  int lineno = 0, rc = RPN_OK;
  while (getline(&line, &cap, f) != -1) {
    lineno++;
    Tokenizer t = make_tokenizer(line, WHITESPACE);
    char *name = next_token(&t);
    if (!name || name[0] == '#') continue;
    if (validate_name(name) != RPN_OK) {
      fprintf(stderr, "  (%s, line %d)\n", path, lineno);
      rc = RPN_ERR;
      continue;
    }
    char *rest = t.current ? t.current : "";
    rest += strspn(rest, WHITESPACE);
    size_t rl = strlen(rest);
    while (rl && strchr(WHITESPACE, rest[rl - 1])) rest[--rl] = '\0';

    int idx = find_word(name);
    if (idx < 0) {
      if (grow() != RPN_OK) { rc = RPN_ERR; break; }
      idx = word_count++;
      memset(&words[idx], 0, sizeof words[idx]);
      snprintf(words[idx].name, MAX_WORD_NAME, "%s", name);
    } else {
      free_word(&words[idx]);
    }
    words[idx].body = strdup(rest);
    if (!words[idx].body) { rc = rpn_err("Out of memory"); break; }
  }
  free(line);
  fclose(f);

  for (int i = 0; i < word_count; ) {
    char *copy = strdup(words[i].body);
    char **toks = malloc((strlen(words[i].body) / 2 + 2) * sizeof *toks);
    int nt = 0;
    if (!copy || !toks) { free(copy); free(toks); rc = rpn_err("Out of memory"); break; }
    Tokenizer t = make_tokenizer(copy, WHITESPACE);
    char *tok;
    while ((tok = next_token(&t))) toks[nt++] = tok;
    Instr *code;
    int ncode;
    int crc = compile(words[i].name, toks, nt, &code, &ncode);
    free(toks);
    free(copy);
    if (crc != RPN_OK) {
      rc = RPN_ERR;
      free_word(&words[i]);
      memmove(&words[i], &words[i + 1], (size_t)(word_count - i - 1) * sizeof *words);
      word_count--;
      continue;
    }
    words[i].code = code;
    words[i].ncode = ncode;
    i++;
  }
  rpn_info("%d word%s loaded from %s", word_count, word_count == 1 ? "" : "s", path);
  return rc;
}
