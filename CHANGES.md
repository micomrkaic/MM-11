# MM-11 2.0 — changes from 1.x

Item numbers refer to the review list.

## Structure (done first; most bugs disappeared in the rewrite)

| # | Change |
|---|---|
| 19 | One dispatch table `ops[]` in `ops.c`: `{name, nargs, fn, category, stack picture, help}`. The table drives dispatch, the arity check, `help`, `see`, tab completion and `COMMANDS.md`. The four parallel word lists and the strcmp chains are gone. |
| 20 | Every op returns `RPN_OK`/`RPN_ERR`. The interpreter snapshots `State` before each top-level token and restores it on error, then skips the rest of the line. |
| 21 | Words are compiled to `Instr[]` at definition time: numbers, op indices, calls resolved by name at run time, and branches. Unknown names are rejected when the word is defined. Recursion is capped at 256 levels and Ctrl-C interrupts. `if/else/then`, `begin/until`, `begin/while/repeat`, `do/loop/+loop` and `i`/`j` were added. |
| 22 | `State` = stack + last x/y + registers + register index + Σ accumulators; all of it is undoable. Undo uses a 64-level ring, a snapshot is pushed only when the state actually changed, and a user word is one step. |
| 23 | Makefile: `-MMD -MP`, separate `build/` and `build-debug/`, `make debug` / `test-debug` with ASan+UBSan, macOS readline/libedit detection, `make docs`, `make install`. |

## Bugs

| # | Fix |
|---|---|
| 1 | `s-` decremented N twice. It is now an exact Welford removal. |
| 2 | x̄/ȳ/σ were stored in the wrong slots. The output is now derived on demand, so there are no slots to mislabel. |
| 3 | Standard deviations are now sample values (n−1), computed with Welford, so there is no negative variance and no NaN. |
| 4 | `s+`/`s-` now take x from X and y from Y (HP order). |
| 5 | `delword` now decrements the word count. |
| 6 | `delword` no longer reads a stale underflow flag; the dispatcher checks arity before the call. |
| 7 | `regidx` range is 0..99 (the off-by-one is gone). |
| 8, 9 | Word bodies are heap-allocated and names are bounded with `snprintf`. There are no fixed body buffers and no `strncpy`. |
| 10 | Runaway recursion is stopped at a depth limit. |
| 11 | Redefining a word replaces it in place. |
| 12 | `parse_number` accepts decimal literals only, so `nan`, `inf` and hex are no longer numbers; one `strtod` call. `1e999` is reported as out of range. |
| 13 | The display builds lines with bounds-checked appends. FIX falls back to SCI for very large or very small values. |
| 14 | Column alignment counts display width; UTF-8 continuation bytes and combining marks are skipped. |
| 15 | Division by zero, domain errors and overflow now report an error and leave the stack unchanged. They no longer silently push 0 or NaN. |
| 16 | The dead `lstx`/`lsty` entries and the duplicate `abs` are gone along with the strcmp chains. |
| 17 | Removed: `my_tokenizer.c` (stray `main`), the `tokenize_and_process` stub, `globals.c`, and the committed `registers.txt`/`user_words.txt`. Config moved from `~/.config/mm_14` to `$XDG_CONFIG_HOME/mm_11`. |
| 18 | Help, `see` and `COMMANDS.md` are generated from the table, and `make test` fails if `COMMANDS.md` is stale. |

## Additions

| # | Addition |
|---|---|
| 24 | `over rot -rot pick depth 2dup 2drop clx` (plus `.` and `.s`). `pick`/`roll` share the depth numbering with the X/Y/Z/T labels. |
| 25 | `sq sqrt cbrt 10^x log2 fact gamma lgamma ncr npr mod floor ceil round min max sign hypot atan2 >p >r gcd lcm e rand seed >hms >h` |
| 26 | `deg rad grad >deg >rad`; exact values at quarter turns in degrees and grads. |
| 27 | `lr` (slope in Y, intercept in X, as on the HP-11C), `yhat`, `xhat`, `mean`, `sdev`, `corr`. The side panel shows a and b live. |
| 28 | `nqnt` uses Acklam's method plus one Halley step, which is full precision. Added `tpdf tcdf tqnt chipdf chicdf chiqnt fpdf fcdf fqnt` (incomplete beta/gamma by continued fractions). Checked against SciPy on 418 cases: worst relative error is 7e-11, in extreme upper-tail quantiles; typical is ~1e-15. |
| 29 | `fix sci eng all`; `spf` cycles through them. X/Y/Z/T labels. A status line shows mode, angle, depth, register and undo depth. |
| 30 | `-e EXPR`, `-f FILE`, stdin when it is not a TTY, `-n`, `--commands`, `--version`. Exit status 1 on error. |
| 31 | `tests/`: 12 golden scripts plus CLI, autoload and docs-freshness checks. They pass under gcc, clang and ASan/UBSan/LSan, and a 400-run random-input fuzz under the sanitizers found nothing. |
| 32 | Words and registers autoload at startup. Readline history persists (1000 lines). XDG-aware paths. Saves are atomic (temporary file, then rename). |
| 33 | Tab completion covers user words, and `>deg`, `%chg` and `||` complete correctly. `see <name>` shows a definition or a built-in's help. |
| 34 | `( ... )` and `\` comments, in definitions and at top level. Definitions and `( )` comments may span lines. |
| 35 | `%` follows the HP: y·x/100, keeping y. `%t` is HP %T (100·x/y) and `%chg` is HP Δ% (100·(x−y)/y), both keeping y. The old `%` computed 100·y/x, which is the reciprocal of %T. |

## Behaviour changes that can affect old word files

- `%` and `%chg` now leave y on the stack.
- `s+` swapped its x/y order.
- `sq`, `sqrt` and similar are now built-ins, so a saved word with the same
  name is rejected on load (with its line number) and the built-in is used
  instead.
