# Odd-Order Magic Square in C (Siamese Method)

This program builds a **magic square of size N×N (odd N only)** using the classic *Siamese method* (aka De la Loubere’s algorithm) and prints it as a grid.

> A *magic square* is a square grid filled with the numbers 1..N² such that every row, column, and the two main diagonals sum to the same constant: **M = N(N²+1)/2**.

---

## 1. Build & Run

```bash
cc magic_square.c -o magic_square
./magic_square
```

The code uses a `#define N 3`. Change it to any odd number (5, 7, 9, …) and recompile.

---

## 2. Quick Demo (N = 3)

```
Magic Square of size 3×3:

 8  1  6 
 3  5  7 
 4  9  2 
```

Each row/col/diagonal sums to 15.

---

## 3. Algorithm: Siamese Method in a Nutshell

1. **Start** with `1` in the **top row, middle column**.
2. For each next number `k = 2..N²`:

   * Move **one row up** and **one column right** ("north-east"). Wrap around edges (top → bottom, right → left).
   * **If that cell is occupied**, move **one row down** instead (from the *original* position of `k-1`).
3. Repeat until all numbers are placed.

This guarantees a valid magic square for all odd `N`.

---

## 4. Code Walkthrough

### 4.1 Globals & Prototypes

```c
#define N 3
void print_square(int square[N][N]);
void generate_magic_square(int square[N][N]);
```

* The square is a fixed-size 2D array.
* You can make it dynamic by allocating `[N][N]` after parsing an argument.

### 4.2 Printing the Square

```c
void print_square(int square[N][N]) {
    printf("Magic Square of size %d×%d:\n\n", N, N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%2d ", square[i][j]);
        }
        printf("\n");
    }
}
```

* Simple nested loops.
* `%2d` pads numbers to keep columns aligned for small N.

### 4.3 Core Generator

```c
void generate_magic_square(int square[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            square[i][j] = 0;     // clear grid

    int num = 1;
    int i = 0;            // start row
    int j = N / 2;        // middle column

    while (num <= N * N) {
        square[i][j] = num;           // place current number

        int newi = (i - 1 + N) % N;   // wrap up
        int newj = (j + 1) % N;       // wrap right

        if (square[newi][newj] != 0) {// occupied?
            i = (i + 1) % N;          // move down one from original
        } else {
            i = newi;                 // NE move accepted
            j = newj;
        }
        num++;
    }
}
```

**Key ideas:**

* `square` initialized to 0 to detect “occupied” cells.
* `%` wraps indices around borders.
* Move logic exactly mirrors Siamese rules.

### 4.4 `main`

```c
int main(void) {
    int square[N][N];
    generate_magic_square(square);
    print_square(square);
    return 0;
}
```