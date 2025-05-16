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
#define _POSIX_C_SOURCE 200809L
#include "tab_complete.h"
#include <stdlib.h>

// Word list for TAB completion
const char *rpn_words[] = {
  "dup", "drop", "swap", "nip", "tuck", "roll",
  "==", "<=", ">=", "!=", "<", ">", "&&", "||", "!",
  "+", "-", "*", "/", "^", "pow", "abs", "inv",
  "sin", "cos", "tan", "asin", "acos", "atan",
  "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
  "log", "ln", "chs", "exp", "%", "%chg",
  "npdf", "ncdf", "nqnt",
  "frac", "int",
  "s+", "s-", "clstat", "statscalc",
  "lstx", "lsty",
  "clst", "u", "p", "q", "help", "sss", "spf", "swapterm",
  "sto", "rcl", "listregs", "clrg", "loadregs", "saveregs","regidx",
  "listwords", "clearwords", "delword", "savewords", "loadwords",
  NULL
};

static char *command_generator(const char *text, int state) {
  static int list_index, len;
  const char *name;
  static char match[64];

  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  while ((name = rpn_words[list_index++])) {
    if (strncmp(name, text, len) == 0) {
      snprintf(match, sizeof(match), "%s", name);
      return strdup(match);
    }
  }
  return NULL;
}

char **rpn_completion(const char *text, int start, int end) {
  (void)start;
  (void)end;
  return rl_completion_matches(text, command_generator);
}
