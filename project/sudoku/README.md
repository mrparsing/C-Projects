# Sudoku Generator & Solver

Generates a **fully‑valid Sudoku**, removes cells according to chosen difficulty (easy / medium / hard) while ensuring a **unique solution**, prints the puzzle, and finally solves it with backtracking.

---

## 1. Features

| Capability                            | Notes                                                                                                           |
| ------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| **Figure‑complete grid generator**    | Recursive backtracking + random shuffling of digits per cell                                                    |
| **Hole‑digger with uniqueness check** | Removes cells until a target blank count is met, verifying only **one** solution remains via a secondary solver |
| Three preset difficulties             | Easy (35 blanks), Medium (45), Hard (55)                                                                        |
| Pretty ASCII board                    | Prints as `+-------+` grid with dots for blanks                                                                 |
| Built‑in solver                       | Backtracking recursion identical to generator but without shuffling                                             |

> Average generation time on a modern CPU is a few milliseconds for easy/medium; hard may take longer because more “dig + verify” iterations are required.

---

## 2. Build & Run

### Linux / macOS

```bash
cc sudoku.c -o sudoku
./sudoku 2   # 1=e, 2=m, 3=h
```

### Windows (MSYS2)

```bash
mingw32-make sudoku.exe   # or gcc sudoku.c -o sudoku.exe
sudoku.exe 3
```

If you omit the argument or supply an invalid difficulty, usage help is printed:

```
Usage: ./sudoku <difficulty>
Difficulty: 1 = easy, 2 = medium, 3 = hard
```

---

## 3. Code Walkthrough

### 3.1 Pretty Printer

`print_sudoku()` outputs a board like:

```
+-------+-------+-------+
| 5 3 . | . 7 . | . . . |
| 6 . . | 1 9 5 | . . . |
| . 9 8 | . . . | . 6 . |
+-------+-------+-------+
| 8 . . | . 6 . | . . 3 |
| 4 . . | 8 . 3 | . . 1 |
| 7 . . | . 2 . | . . 6 |
+-------+-------+-------+
| . 6 . | . . . | 2 8 . |
| . . . | 4 1 9 | . . 5 |
| . . . | . 8 . | . 7 9 |
+-------+-------+-------+
```

A dot (`.`) marks an empty cell.

### 3.2 Generator Pipeline

1. **`init_grid()`** zero‑fills a 9×9 array then calls `fill_grid()`.
2. **`fill_grid()`** recursively searches the first empty cell, shuffles an array `1..9`, and tries each digit that satisfies `is_safe()`. Stops when the board is full.
3. **`dig_holes()`** randomly blanks cells until the target count is reached *and* uniqueness remains:

   * Picks a random non‑zero cell, temporarily zeroes it.
   * Copies the board into `copy[][]` and counts solutions via `count_solutions()`.
   * If count ≠ 1, restores the value; else decrements `holes`.

### 3.3 Safety Check

`is_safe()` verifies standard Sudoku constraints (row, column, 3×3 box) by brute‑force scanning—fast enough for 9×9.

### 3.4 Solver

`solve_grid()` is a classic backtracking algorithm (no heuristics, simplest cell‑first). Adequate thanks to pre‑filtered puzzle; complexity \~O(b^n) worst‑case but most boards solve in microseconds.

---

## 4. Difficulty Table

| Level      | Target blanks (holes) | Expected solving difficulty              |
| ---------- | --------------------- | ---------------------------------------- |
| 1 (Easy)   | 35                    | Many givens, low branching               |
| 2 (Medium) | 45                    | Requires deeper search                   |
| 3 (Hard)   | 55                    | Minimal clues, often near‑minimal Sudoku |