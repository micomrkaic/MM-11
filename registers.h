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
#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdbool.h>
#include "stack.h"

#define MAX_REGS 26  // one for each letter a–z
extern double registers[MAX_REGS];
extern bool reg_used[MAX_REGS];  // track which registers are initialized

void clear_registers(void);
int save_registers_to_file(void);
int load_registers_from_file(void);
void list_registers(void);
void top_to_register(Stack *stack);
void register_to_top(Stack *stack);
void set_reg_index(Stack* stack);

#endif // REGISTERS_H
