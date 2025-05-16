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
#ifndef TOKENIZER_H
#define TOKENIZER_H

#define MAX_RECURSION_DEPTH 8

typedef struct {
  char *input;
  char *current;
  const char *delim;
} Tokenizer;

Tokenizer make_tokenizer(char *input, const char *delim);
char *next_token(Tokenizer *t);
void tokenize_and_process(char *line, int depth);

#endif // TOKENIZER_H
