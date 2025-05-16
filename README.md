# 🧮 RPN Calculator — `MM-11`

A fast, extensible reverse Polish notation (RPN) calculator written in C, inspired by the great HP RPN calculators of the past. Supports real arithmetic, stack operations, user-defined words, statistical registers, and more.

---

## 📦 Features

- Stack-based evaluation using reverse Polish notation
- Real-number arithmetic with precision control
- Unary and binary mathematical operations
- Named registers `a`–`z` for storing values
- Persistent register and word saving/loading
- User-defined words/macros
- Undo history
- Statistical register support with `Σx`, `Σy`, `σ`, and `ρ`
- Normal distribution functions: `npdf`, `ncdf`, `nqnt`
- Built-in `help` screen with syntax guide
- Tab-completion using GNU Readline
- Modular tokenizer implementation

---

## 🧑‍💻 Usage

Start the program with **bash ./calc**

Enter expressions like
3 4 +         # Adds 3 and 4
dup           # Duplicates top of stack
sto a         # Stores top of stack to register 'a'
rcl a         # Recalls value from register 'a'
: inc2 2 + ;  # Defines a word 'inc2' that adds 2
inc2          # Executes the word


---

## ✨ Commands

- `+`, `-`, `*`, `/` — Basic arithmetic
- `^`, `pow` — Power function
- `dup` — Duplicate the top of the stack
- `drop` — Drop the top element
- `swap` — Swap top two elements
- `nip` — Remove second item
- `tuck` — Duplicate top and insert below second
- `sto x` — Store top value to register `x`
- `rcl x` — Recall value from register `x`
- `: name ... ;` — Define a new word
- `u` — Undo last operation
- `p` — Set decimal precision (top of stack)
- `c` — Clear the stack
- `clrg` — Clear all registers
- `loadregs` — Load registers from file
- `saveregs` — Save registers to file
- `listregs` — List non-empty registers
- `clearwords` — Remove all user-defined words
- `savewords` — Save words to file
- `loadwords` — Load words from file
- `listwords` — List all defined words
- `help` — Show help screen
- `q` — Quit the calculator

## 📊 Statistical Functions

- `s+` — Add an (x, y) pair to statistics
- `s-` — Remove an (x, y) pair
- `clstat` — Clear all statistical registers
- `statscalc` — Print means, std devs, and correlation
- `lstx`, `lsty` — Recall last x/y from binary operations


| Function | Description                          |
|----------|--------------------------------------|
| `s+`     | Add x,y pair to running stats        |
| `s-`     | Remove x,y pair                      |
| `clstat` | Clear statistical registers          |
| `statscalc` | Print summary stats (mean, std, ρ) |

---

## 🔧 Building and Dependencies

You will need GNU readline library to compile and link this. To compile just use the makefile.

---

## 🧱 File Structure

| File                 | Description                         |
|----------------------|-------------------------------------|
| `main.c`             | Main REPL and command loop          |
| `tokenizer.c/h`      | Custom reentrant tokenizer          |
| `stack.c/h`          | Stack implementation                |
| `registers.c/h`      | Manipulating Registers              |
| `tab_complete.c/h`   | Tab completion handler (readline)   |
| `words.c/h`          | User-defined word management        |
| `my_math.c/h`        | Math operations                     |
| `my_statistics.c/h`  | Statistical functions               |
| `stack_printing.c/h` | Functions to print the results      |

---

## 🗃️ Save Registers and User-Defined Macros

- Registers and user words are saved in plain text format
- Can be restored across sessions

---

## 🧠 More to Come in Other Calculators

- Complex numbers and matrix support
- Strings, lists, unit conversion
- Inline `if` / `loop` constructs
- Graphing (text mode)
- Symbolic operations
