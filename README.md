# MM-11 — a simple RPN calculator

A reverse Polish notation calculator in C17, in the spirit of the HP-11C, with a
Forth-like word system. Its features:

- real arithmetic, scientific and probability functions
- 100 registers
- HP-style Σ statistics with linear regression
- user-defined words with `if`/`do`/`begin` control flow
- 64-level undo
- a scriptable batch mode

## Build and install

```
make                 # ./mm_11 (needs GNU readline; macOS uses Homebrew readline or libedit)
make test            # golden-file tests
make debug           # ./mm_11-debug with AddressSanitizer + UBSan
make test-debug      # tests under the sanitizers
make docs            # regenerate COMMANDS.md
make install         # to ~/.local: bin/mm_11, share/man/man1/mm_11.1, share/doc/mm_11/
make uninstall       # remove those
```

`PREFIX` defaults to `~/.local`, so no root is needed; `sudo make install
PREFIX=/usr/local` installs system-wide. Make sure `~/.local/bin` is in your
`PATH`. `man mm_11` works once `~/.local/share/man` is on the man path, which
most distributions handle automatically when the directory exists.

The build uses `-Werror`, so a new compiler release can turn a fresh warning
into a build failure. If that happens, `make WARN=-Wall` builds without it.

## Use

```
$ ./mm_11                      # interactive
> 3 4 +
> deg 30 sin
> : hyp ( a b -- c ) sq swap sq + sqrt ;
> 3 4 hyp

$ ./mm_11 -e "2 10 ^"          # one-shot
1024.000000
$ echo "all 1 3 /" | ./mm_11   # pipe
0.333333333333333
$ ./mm_11 -f script.rpn        # run a file
```

Batch runs (`-e`, `-f`, or stdin that is not a terminal) print the final stack
one value per line. They exit with status 1 if any error occurred. Pass `-n` to
skip autoloading saved words and registers.

The full command list is in [COMMANDS.md](COMMANDS.md), which is generated from
the dispatch table, and `man mm_11` covers the same ground. In the calculator,
`help` lists every command and `see <name>` describes one.

## Behaviour worth knowing

- **Stack labels.** The bottom four levels show as `X Y Z T`, and deeper levels
  by their depth. `pick` and `roll` use the same depth numbers: `0 pick` is
  `dup`, `1 roll` is `swap`, `2 roll` is `rot`.
- **Errors are atomic.** A failing token leaves everything as it was before
  that token, the same way an HP shows "Error". This covers the stack, last
  x/y, the registers and the statistics. The rest of the line is skipped. A
  user word counts as one token.
- **Undo.** `u` steps back through up to 64 changes. Settings, such as the
  display mode, are not part of undo. Neither are word definitions.
- **Registers.** `n regidx` selects a register, and then `sto`, `sto+`,
  `sto-`, `sto*`, `sto/` and `rcl` act on it. As on the HP, `sto` leaves X in
  place.
- **Statistics.** `s+` takes x from X and y from Y (HP order). The standard
  deviations are sample (n−1) values. The accumulators use Welford's method, so
  `s-` is exact and large offsets don't destroy precision. The side panel
  updates live: N and the sums on one side; x̄, sₓ, ȳ, sᵧ, r and the
  regression a, b on the other.
- **Percent (HP semantics).** `%` gives y·x/100, `%t` gives 100·x/y and `%chg`
  gives 100·(x−y)/y. All three leave y in Y.
- **Angles.** `deg`, `rad` and `grad` set the mode. In degrees and grads,
  multiples of 90°/100g give exact results, so `180 sin` is 0. `>p`/`>r`
  convert between rectangular and polar coordinates, and `>hms`/`>h` between
  decimal hours and H.MMSS.
- **Display.** The modes are `fix`, `sci`, `eng` (exponent a multiple of 3) and
  `all` (15 significant digits). `n p` sets the number of digits. FIX switches
  to SCI when a number is too large or too small to show.

## User words

```
: name body ;            \ may span several lines
( comment )   \ comment  \ parentheses need surrounding spaces
see name                 \ show the definition
listwords  n delword  clearwords  savewords  loadwords
```

- **Definition-time checks.** Each word is compiled when it is defined, so an
  unknown name is reported then rather than at run time. A word may call
  itself.
- **Late binding.** Calls are resolved by name, so redefining a word also
  changes what its callers do.
- **Limits.** Recursion stops at 256 levels. Ctrl-C interrupts a running word,
  and the stack rolls back.

Control flow works inside definitions only:

```
flag if ... [else ...] then
begin ... flag until
begin ... flag while ... repeat
limit start do ... i ... loop        \ skipped when start = limit
limit start do ... n +loop           \ negative n counts down to limit inclusive
```

```
: fac  dup 1 > if dup 1 - fac * then ;
: sum  ( n -- 1+..+n ) 0 swap 1 + 1 do i + loop ;
```

## Files

Words, registers and readline history live in `$XDG_CONFIG_HOME/mm_11/`, which
defaults to `~/.config/mm_11/`. Saved words and registers load automatically at
startup. Saving writes to a temporary file and then renames it, so an
interrupted save can't corrupt the existing file. Old MM-11 files in
`~/.config/mm_14/` use the same format: copy `user_words.txt` to `words.txt`,
and `registers.txt` as is.

| File | Contents |
|---|---|
| `main.c` | argument handling, REPL, batch mode |
| `interp.c` | line interpreter: comments, definitions, atomic tokens, undo |
| `ops.c` | **dispatch table** and all built-in operations |
| `words.c` | word compiler, executor, persistence |
| `state.c` | `State` (everything undoable), settings, undo ring, messages |
| `stack.c` | push/pop/peek |
| `registers.c` | registers and their file |
| `my_math.c` | distributions, combinatorics, angles, H.MS, RNG |
| `my_statistics.c` | Welford accumulators and derived statistics |
| `stack_printing.c` | number formatting and the stack display |
| `help_functions.c` | help, `see`, COMMANDS.md — all from the table |
| `tab_complete.c` | readline completion over commands and user words |
| `tokenizer.c` | reentrant tokenizer |
| `util_func.c` | number parsing, config paths |
| `mm_11.1` | man page |
| `tests/` | `*.rpn` scripts with expected `*.out`; `run.sh` |

## Adding a command

Write `static int op_x(State *s)` in `ops.c` and add one row to `ops[]`, giving
its name, minimum stack depth, category, stack picture and description. The
dispatcher checks arity, and help, completion and `see` pick the command up
automatically. Run `make docs` and `UPDATE=1 tests/run.sh ./mm_11` afterwards,
then review the diff, and add the name to the category list in `mm_11.1`.

## Still to come in later calculators

- **MM-15:** complex numbers, matrices, numerical integration and root finding.
- **MM-48:** terminal plotting, symbolic algebra, strings, lists, units.

## Licence

GPL-3.0-or-later.
