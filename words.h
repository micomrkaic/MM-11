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
#ifndef WORDS_H
#define WORDS_H

#include <stdbool.h>
#include "state.h"

typedef enum {
  I_NUM, I_OP, I_CALL, I_BRANCH, I_BRANCH0, I_DO, I_LOOP, I_PLOOP, I_I, I_J
} InstrKind;

typedef struct {
  InstrKind k;
  union {
    double num;     /* I_NUM */
    int    op;      /* I_OP: index into ops[] */
    int    target;  /* branches, I_DO (exit), I_LOOP/I_PLOOP (body start) */
    char  *name;    /* I_CALL: late-bound by name, so redefinitions take effect */
  } u;
} Instr;

typedef struct {
  char   name[MAX_WORD_NAME];
  char  *body;      /* normalized source, for listing/saving/see */
  Instr *code;      /* compiled at definition time */
  int    ncode;
} UserWord;

extern UserWord *words;
extern int word_count;

int  find_word(const char *name);                       /* index or -1 */
int  define_word(const char *name, char **toks, int ntok);
int  exec_word(State *s, int idx, int depth);
void list_words(void);
int  see_word(const char *name);
int  delete_word(int idx);
int  clear_words(void);
int  save_words(void);
int  load_words(bool quiet_if_missing);
void free_words(void);
bool is_control_word(const char *t);

#endif
