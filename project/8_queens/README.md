# N-Queens Solver (8×8 by default)

Backtracking classic: place **N queens** on an N×N chessboard so none attack each other (no same row, column, or diagonal). This program:

* Uses recursion and backtracking to explore all valid boards.
* Prints every solution as an ASCII grid (`Q` = queen, `.` = empty).
* Counts total solutions and prints the final tally.

Default board size is 8 (via `#define N 8`).

---

## 1. Build & Run

```bash
cc nqueens.c -o nqueens
./nqueens
```

Expect a lot of output (92 solutions for N=8). Pipe to less if needed:

```bash
./nqueens | less
```

---

## 2. Code Walkthrough

### 2.1 Representation: `queens[]`

```c
int queens[N]; // queens[row] = col index of the queen in that row
```

One integer per row. Since each row must contain exactly one queen, you only store the column. This automatically enforces the "one queen per row" rule.

### 2.2 Printing a Board

```c
void print_chessboard(int queens[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf(queens[i] == j ? "Q " : ". ");
        printf("\n");
    }
    printf("\n");
}
```

Row by row, compare `j` to `queens[i]`. If equal, draw a queen.

### 2.3 Safety Check

```c
int is_safe(int queens[N], int row, int col) {
    for (int i = 0; i < row; i++) {
        if (queens[i] == col) return 0;                     // same column
        if (abs(queens[i] - col) == row - i) return 0;      // same diagonal
    }
    return 1;
}
```

* **Column clash**: if any previous queen shares `col`.
* **Diagonal clash**: two squares are on the same diagonal if `|Δrow| == |Δcol|`. Here `row - i` is Δrow, `abs(queens[i] - col)` is Δcol.

### 2.4 Backtracking Core

```c
void solve(int queens[N], int row) {
    if (row == N) {
        solutions++;
        print_chessboard(queens);
        return;
    }
    for (int col = 0; col < N; col++) {
        if (is_safe(queens, row, col)) {
            queens[row] = col;
            solve(queens, row + 1);
        }
    }
}
```

* Place a queen in the current `row` at every column that passes `is_safe`.
* Recurse to the next row.
* No explicit "undo" needed because you overwrite `queens[row]` in the next iteration.

### 2.5 Entry Point

```c
int main(void) {
    int queens[N];
    solve(queens, 0);
    printf("Found %d solutions.\n", solutions);
    return 0;
}
```

Starts at row 0, prints total.

---

## 3. Complexity & Performance

* Worst case is exponential: roughly O(N!). Backtracking prunes tons of branches, but growth is still explosive.
* For N=8, there are 92 solutions. For larger N, printing them all becomes huge (N=14 → 365,596 solutions).