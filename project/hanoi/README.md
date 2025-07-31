# Tower of Hanoi

Classic recursive solution to the **Tower of Hanoi**. Disks are stored in three stacks (`Tower A/B/C`) inside a `State` struct; each move pops from one stack and pushes to another.

---

## 1. Build & Run

```bash
chmod +x build.sh
./build
./hanoi            # solves for n = 3 (default in code)
```

Edit `int n = 3;` in `main()` to change the number of disks (≤ `MAX_DISKS`, default 64).

---

## 2. Data Structures

```c
typedef struct {
    int disks[MAX_DISKS];
    int top;            // index of last element, -1 if empty
} Tower;

typedef struct {
    Tower A, B, C;      // the three towers
} State;
```

* Each tower is just a fixed-size stack.
* Disk sizes are integers: larger number ⇒ larger disk.
* In the initial state, tower **A** holds `num_disks .. 1` (bottom → top).

---

## 3. Core Helpers

### 3.1 Initialization

```c
void init_state(State *s, int n) {
    s->A.top = n - 1; s->B.top = s->C.top = -1;
    for (int i = 0; i < n; ++i)
        s->A.disks[i] = n - i;   // biggest at index 0
}
```

### 3.2 Stack Operations

```c
void push(Tower *t, int disk) { t->disks[++t->top] = disk; }
int  pop (Tower *t)           { return t->disks[t->top--]; }
```

Checks for overflow/underflow and exits on error.

### 3.3 Move + Print

```c
void move_disk(State *s, char from, char to) {
    Tower *src = (from=='A'?&s->A:from=='B'?&s->B:&s->C);
    Tower *dst = (to  =='A'?&s->A:to  =='B'?&s->B:&s->C);
    int disk = pop(src);
    push(dst, disk);
    printf("Move disk %d from %c to %c\n", disk, from, to);
}
```

`print_state()` dumps each tower in order: bottom→top as stored.

---

## 4. Recursive Algorithm

```c
void solve_hanoi(State *s, int n, char from, char to, char aux) {
    if (n == 1) {
        move_disk(s, from, to);
        print_state(s);
        return;
    }
    solve_hanoi(s, n-1, from, aux, to); // move n-1 to auxiliary
    move_disk(s, from, to);             // move biggest to target
    print_state(s);
    solve_hanoi(s, n-1, aux, to, from); // move n-1 on top of biggest
}
```

**Base case**: one disk → just move.

**Induction**: to move `n` disks:

1. Move top `n-1` from `from` to `aux` using `to` as helper.
2. Move the largest disk from `from` to `to`.
3. Move the `n-1` stack from `aux` to `to` using `from` as helper.

This yields `2^n - 1` moves.

---

## 5. Sample Output (n = 3)

```
Initial state:
Tower A: 3 2 1 
Tower B: 
Tower C: 

Move disk 1 from A to C
Tower A: 3 2 
Tower B: 
Tower C: 1 

Move disk 2 from A to B
Tower A: 3 
Tower B: 2 
Tower C: 1 

Move disk 1 from C to B
Tower A: 3 
Tower B: 2 1 
Tower C: 

Move disk 3 from A to C
Tower A: 
Tower B: 2 1 
Tower C: 3 

Move disk 1 from B to A
Tower A: 1 
Tower B: 2 
Tower C: 3 

Move disk 2 from B to C
Tower A: 1 
Tower B: 
Tower C: 3 2 

Move disk 1 from A to C
Tower A: 
Tower B: 
Tower C: 3 2 1 
```