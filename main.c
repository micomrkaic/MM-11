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
/* Mico's simple RPN Calculator -- MM-11

   Still to do as of May 16, 2025

   for MM-11
   . loops and if-then constructs using token buffering
   . run, load and save programs
   . documentation -- README.md file etc.

   for MM-15
   . complex number support (for mm-15)
   . matrices
   . numerical integration and zero finding
   . checks if any terms words; definition are undefined
   . limits on the depth of recursion. Implement by fully expanding the definitions right away

   for MM-48
   1. simple plotting inside the terminal??
   2. symbolic stuff
   3. strings
   4. lists
   5. units and constants
   6. printing and prompting commands
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "globals.h"
#include "stack.h"
#include "my_math.h"
#include "stack_printing.h"
#include "my_statistics.h"
#include "words.h"
#include "help_functions.h"
#include "registers.h"
#include "util_func.h"
#include "tab_complete.h"
#include "tokenizer.h"

const char *unary_ops[] = {
    "sin", "cos", "tan","asin", "acos", "atan", "sinh", "cosh", "tanh","asinh", "acosh", "atanh",
    "log", "ln", "chs", "exp", "inv", "abs", "!", "npdf", "ncdf", "nqnt","lstx", "lsty","frac", "int",
    NULL
};

const char *binary_ops[] = {
    "+", "-", "*", "/", "^", "pow", "==", ">=", ">", "<", "<=", "!=", "&&", "||", "%", "%chg", "s+", "s-",
    NULL
};

bool is_unary_operator(const char *token) {
  for (int i = 0; unary_ops[i] != NULL; i++) {
    if (strcmp(token, unary_ops[i]) == 0)
      return true;
  }
  return false;
}

bool is_binary_operator(const char *token) {
  for (int i = 0; binary_ops[i] != NULL; i++) {
    if (strcmp(token, binary_ops[i]) == 0)
      return true;
  }
  return false;
}

// Perform binary operation
double binaryOp(double a, double b, const char* op) {
  last_y = a;
  last_x = b;
  
  if (op[0] == '+') return a + b;
  if (op[0] == '-') return a - b;
  if (op[0] == '*') return a * b;
  if (op[0] == '/') return safe_div(a,b);
  if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) return pow(a, b);
  if (strcmp(op, "==") == 0) return (a==b);
  if (strcmp(op, "<=") == 0) return (a<=b);
  if (strcmp(op, "<") == 0) return (a<b);
  if (strcmp(op, ">") == 0) return (a>b);
  if (strcmp(op, ">=") == 0) return (a>=b);
  if (strcmp(op, "!=") == 0) return (a!=b);
  if (strcmp(op, "&&") == 0) return (a && b);
  if (strcmp(op, "||") == 0) return (a || b);
  if (strcmp(op, "%") == 0) return (a/b*100.0);
  if (strcmp(op, "%chg") == 0) return ((b-a)/a*100.0);
  if (strcmp(op, "s+") == 0) return sigma_plus(a,b);
  if (strcmp(op, "s-") == 0) return sigma_minus(a,b); 
  printf("Invalid operator!\n");
  return 0;
}

// Perform unary operation
double unaryOp(double a, const char* op) {
  last_x = a;  // Save the X register

  if (strcmp(op, "frac") == 0) return safe_frac(a);
  if (strcmp(op, "int") == 0) return safe_int(a);
  if (strcmp(op, "nqnt") == 0) return inverse_normal_cdf(a,1.0e-8);
  if (strcmp(op, "npdf") == 0) return normal_pdf(a);
  if (strcmp(op, "ncdf") == 0) return normal_cdf(a);
  if (strcmp(op, "abs") == 0) return fabs(a);
  if (strcmp(op, "!") == 0) return !(a);
  if (strcmp(op, "abs") == 0) return fabs(a);
  if (strcmp(op, "sin") == 0) return sin(a);
  if (strcmp(op, "cos") == 0) return cos(a);
  if (strcmp(op, "tan") == 0) return tan(a);
  if (strcmp(op, "asin") == 0) return asin(a);
  if (strcmp(op, "acos") == 0) return acos(a);
  if (strcmp(op, "atan") == 0) return atan(a);
  if (strcmp(op, "sinh") == 0) return sinh(a);
  if (strcmp(op, "cosh") == 0) return cosh(a);
  if (strcmp(op, "tanh") == 0) return tanh(a);
  if (strcmp(op, "asinh") == 0) return asinh(a);
  if (strcmp(op, "acosh") == 0) return acosh(a);
  if (strcmp(op, "atanh") == 0) return atanh(a);
  if (strcmp(op, "log") == 0) return safe_log10(a);
  if (strcmp(op, "ln") == 0) return safe_ln(a);
  if (strcmp(op, "inv") == 0) return safe_div(1.0,a);
  if (strcmp(op, "chs") == 0) return -a;
  if (strcmp(op, "exp") == 0) return exp(a);
  printf("Invalid function!\n");
  return 0;
}

void process_token(char* token, Stack* stack, Stack* old_stack) {
  if (strcmp(token, "q") == 0)          { terminate_program = true; return; } // Quit
  if (strcmp(token, "u") == 0)          { copyStack(stack, old_stack); return; } // Undo

  copyStack(old_stack, stack); // Save the stack before processign the next token
  if (isNumber(token))                  { push(stack, atof(token)); return; }  // Push a single number
  if (strcmp(token, "clst") == 0)       { initStack(stack); return; } // Clear stack
  if (strcmp(token, "sss") == 0)        { show_stat_regs = !show_stat_regs; return; } // Show statistics
  if (strcmp(token, "swapterm") == 0)   { small_terminal = !small_terminal; return;} // Small<->Big terminal output
  if (strcmp(token, "delword") == 0)    { if (!stack_underflow) delete_word_by_index((int)pop(stack)); return; }
  if (strcmp(token, "savewords") == 0)  { save_words_to_file(); return; }
  if (strcmp(token, "loadwords") == 0)  { load_words_from_file(); return; }
  if (strcmp(token, "spf") == 0)        { fixed_point_format = !fixed_point_format; return; }
  if (strcmp(token, "clstat") == 0)     { sigma_clear(); return; }
  if (strcmp(token, "statscalc") == 0)  { sigma_evaluate(); return; }
  if (strcmp(token, "listregs") == 0)   { list_registers(); return; }
  if (strcmp(token, "listwords") == 0)  { list_words(); return;  }
  if (strcmp(token, "clrg") == 0)       { clear_registers(); return; }
  if (strcmp(token, "saveregs") == 0)   { save_registers_to_file(); return; }
  if (strcmp(token, "loadregs") == 0)   { load_registers_from_file(); return; }
  if (strcmp(token, "clearwords") == 0) { clear_words(); return;  }
  if (strcmp(token, "help") == 0)       { help_screen(); return; }
  if (strcmp(token, "pi") == 0)         { push(stack, M_PI); return; }
  if (strcmp(token, "lstx") == 0)       { push(stack, last_x); return; }
  if (strcmp(token, "lsty") == 0)       { push(stack, last_y); return; }
  if (strcmp(token, "sto") == 0)        { top_to_register(stack); return; }
  if (strcmp(token, "rcl") == 0)        { register_to_top(stack); return;}
  if (strcmp(token, "p") == 0)          { set_disp_precision(stack); return;}
  if (strcmp(token, "regidx") == 0)     { set_reg_index(stack); return;}

  // Binary functions
if (is_binary_operator(token))
  {
    if (stack->top < 1) {
      printf("Need at least 2 numbers on stack!\n");
    } else {
      double b = pop(stack);
      double a = pop(stack);
      double result = binaryOp(a, b, token);
      push(stack, result);
    }
  }
else if (is_unary_operator(token))
  {
    if (isEmpty(stack)) {
      printf("Need at least 1 number on stack!\n");
    } else {
      double a = pop(stack);
      double result = unaryOp(a, token);
      push(stack, result);
    }
  }
  
  else if (strcmp(token, "dup") == 0) stack_dup(stack);
  else if (strcmp(token, "swap") == 0) stack_swap(stack);
  else if (strcmp(token, "drop") == 0) stack_drop(stack);
  else if (strcmp(token, "nip") == 0) stack_nip(stack);
  else if (strcmp(token, "tuck") == 0) stack_tuck(stack);
  else if (strcmp(token, "roll") == 0) stack_roll(stack);
 
  // User-defined word?
else {
    UserWord* w = find_word(token);
    if (w) {
        char* body = strdup(w->body);
        Tokenizer t = make_tokenizer(body, " ");
        char* sub;

        while ((sub = next_token(&t)) != NULL) {
	  //            printf("token=%s\n", sub);
            process_token(sub, stack, old_stack);
        }

        free(body);
    } else {
        printf("Invalid input: %s\n", token);
    }
 }
}

int main() {
  Stack stack;
  Stack old_stack;
  char* input = NULL;

  splash_screen();
  rl_attempted_completion_function = rpn_completion;

  initStack(&stack);
  copyStack(&old_stack, &stack);
  
  while ((input = readline("> "))) {
    if (!input || strlen(input) == 0) {
      if (small_terminal)
	printStack_small_terminal(&stack, precision,"Stack");
      else
	printStack_big_terminal(&stack, precision);
      free(input);
      continue;
    }

    if (*input) add_history(input);

    Tokenizer t = make_tokenizer(input, " ");
    char *token;

    while ((token = next_token(&t))) {
      if (strcmp(token, ":") == 0) { // A definition of a new word
        char *name = next_token(&t);
        if (!name || word_count >= MAX_WORDS) {
	  printf("Invalid or too many word definitions.\n");
        } else {
	  UserWord *w = &words[word_count++];
	  strncpy(w->name, name, MAX_WORD_NAME);
	  w->body[0] = '\0';
	  
	  char *def_token;
	  while ((def_token = next_token(&t))) {
	    if (strcmp(def_token, ";") == 0) break;
	    strncat(w->body, def_token, MAX_WORD_BODY - strlen(w->body) - 2);
	    strcat(w->body, " ");
	  }	  
	  printf("Defined word: %s => %s\n", w->name, w->body);
        }
      } else {
        process_token(token, &stack, &old_stack);
      }
    }
    
    if (small_terminal) {
      printStack_small_terminal(&stack, precision,"Stack");
    } else {
      printStack_big_terminal(&stack, precision);
    }
    if (terminate_program) goto nice_exit;
  }

 nice_exit:
  free(input);
  clear_history();  // frees history list memory
  printf("Goodbye!\n");
  return 0;
}
