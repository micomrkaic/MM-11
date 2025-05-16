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
#include <stdio.h>

void help_screen(void) {
  printf("All input is case sensitive.\n");
  printf("+ - * / abs inv chs %% %%chg frac int\n");
  printf("sin cos tan asin acos atan\n");
  printf("sinh cosh tanh asinh acosh atanh\n");
  printf("ln log exp ^ pow npdf ncdf nqnt\n");
  printf("s+ s- clstat statscalc\n");
  printf("clst dup drop swap nip tuck roll lstx lsty\n");
  printf("== <= >= != < > && || ! [tests & logic]\n");
  printf("sto rcl regidx listregs clrg loadregs saveregs\n");
  printf("listwords clearwords delword savewords loadwords\n");
  printf("u [undo] p {1-9} [precision]\n");
  printf("sss [stats on<->off] spf [SCI<->ENG]\n");
  printf("help q or ctrl-d [quit] \n");
  printf("\n\n");
}
void splash_screen(void) {
  const char* BOLD = "\033[1m";
  const char* RESET = "\033[0m";

  printf(" \n**************** Mico's simple RPN Calculator ****************\n");
  printf(" Enter these commands for action. All input is case sensitive.\n");
  printf("   %snumber%s [limited to real numbers for now] -- push to stack\n", BOLD, RESET);
  printf("   %sdup%s, %sdrop%s, %sswap%s, %snip%s, %stuck%s, %sroll%s -- stack operations\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %ssto%s x -- store top of stack to register 'x'\n", BOLD, RESET);
  printf("   %srcl%s x -- recall value from register 'x' to stack\n", BOLD, RESET);
  printf("   %slistregs%s  -- list the contents of occupied registers\n", BOLD, RESET);
  printf("   %sclrg%s -- clear registers\n", BOLD, RESET);
  printf("   %s+%s, %s-%s, %s*%s, %s/%s, %sinv%s -- the usual arithmetic stuff\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %sfrac%s -- the fractional part\n", BOLD, RESET);
  printf("   %sint%s -- the integerpart\n", BOLD, RESET);
  printf("   %s^%s or %spow%s -- (x^y)\n", BOLD, RESET, BOLD, RESET);
  printf("   %s%%%s, %s%%%schg%s  -- percent and percent change\n", BOLD, RESET, BOLD, RESET, RESET);
  printf("   %ssin%s, %scos%s, %stan%s -- trigonometric (arguments in radians)\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %sasin%s, %sacos%s, %satan%s -- inverse trigonometric\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %ssinh%s, %scosh%s, %stanh%s -- hyperbolic\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %sasinh%s, %sacosh%s, %satanh%s -- inverse hyperbolic\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %slog%s, %sln%s -- logarithmic (base 10, natural)\n", BOLD, RESET, BOLD, RESET);
  printf("   %sexp%s -- exponential (e^x)\n", BOLD, RESET);
  printf("   %snpdf%s, %sncdf%s, %snqnt%s -- normal pdf, cdf, and quantiles respectively\n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %schs%s -- negate top number\n", BOLD, RESET);
  printf("   %sabs%s -- absolute value of top number\n", BOLD, RESET);
  printf("   %s==%s, %s!=%s, %s<=%s, %s<%s, %s>%s, %s>=%s, %s&&%s, %s||%s, %s!%s -- comparisons and logicals \n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET,
         BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %ss+%s, %ss-%s, %sclearstats%s, %sstatscalc%s -- simple statistical operations a la HP-11 \n",
         BOLD, RESET, BOLD, RESET, BOLD, RESET, BOLD, RESET);
  printf("   %sp%s N -- set precision to N decimals (0-9)\n", BOLD, RESET);
  printf("   %sclst%s -- clear stack\n", BOLD, RESET);
  printf("   %ssss%s -- flip the flag to print of statistical registers\n", BOLD, RESET);
  printf("   %sspf%s -- flip the flag for fixed point output\n", BOLD, RESET);
  printf("   %su%s -- undo last operation\n", BOLD, RESET);
  printf("   %shelp%s -- repeat these instructions\n", BOLD, RESET);
  printf("   Operations with user-defined words \n");
  printf("   listwords, delword, savewords, loadwords \n");  
  printf("   %sq%s - or Ctrl-d -- quit\n", BOLD, RESET);
  printf("****************************************************************\n\n");
}

