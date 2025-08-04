# Stack and Queue

This project contains two basic implementations in C of the data structures **stack** and **queue**.

---

## Data Structures Implemented

### 1. Stack (LIFO)

- Structure: singly linked list.
- Behavior: **Last-In, First-Out** — the last element added is the first one to be removed.
- Operations:
  - `push(value)`: adds a value to the top of the stack.
  - `pop()`: removes and returns the value at the top of the stack.
  - `peek()`: returns the value at the top **without removing it**.
  - `is_empty()`: returns 1 if the stack is empty, 0 otherwise.
  - `print_stack()`: displays the stack contents from top to bottom.

---

### 2. Queue (FIFO)

- Structure: singly linked list.
- Behavior: **First-In, First-Out** — the first element added is the first one to be removed.
- Operations:
  - `enqueue(value)`: adds a value to the rear of the queue.
  - `dequeue()`: removes and returns the value at the front of the queue.
  - `print_queue()`: displays all queue contents from front to rear.
  - `is_empty()`: returns 1 if the queue is empty, 0 otherwise.

---

## How to Run

Compile the desired file using `gcc`:

```bash
gcc stack.c -o stack
./stack