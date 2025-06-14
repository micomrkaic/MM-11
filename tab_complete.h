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
#ifndef TAB_COMPLETE_H
#define TAB_COMPLETE_H

// Include required headers
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

// Function declarations
char **rpn_completion(const char *text, int start, int end);

#endif // TAB_COMPLETE_H
