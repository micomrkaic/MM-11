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

#define MAX_WORDS 32
#define MAX_WORD_NAME 32
#define MAX_WORD_BODY 256

typedef struct {
  char name[MAX_WORD_NAME];
  char body[MAX_WORD_BODY];
} UserWord;

extern UserWord words[MAX_WORDS];
extern int word_count;

// Words functions
void list_words(void);
int delete_word_by_index(int index);
void clear_words(void);
int save_words_to_file(void);
int load_words_from_file(void);
UserWord* find_word(const char* name);

#endif // WORDS_H
