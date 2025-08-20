#include <stdio.h>
#include <stdlib.h>

#define MAX_DISKS 64

typedef struct
{
    int disks[MAX_DISKS];
    int top;
} Tower;

typedef struct
{
    Tower A, B, C;
} State;

void init_state(State *state, int num_disks)
{
    state->A.top = num_disks - 1;
    state->B.top = -1;
    state->C.top = -1;

    for (int i = 0; i < num_disks; ++i)
    {
        state->A.disks[i] = num_disks - i;
    }
}

void push(Tower *t, int disk)
{
    if (t->top >= MAX_DISKS - 1)
    {
        fprintf(stderr, "Error: Tower overflow!\n");
        exit(EXIT_FAILURE);
    }
    t->disks[++(t->top)] = disk;
}

int pop(Tower *t)
{
    if (t->top < 0)
    {
        fprintf(stderr, "Error: Tower underflow!\n");
        exit(EXIT_FAILURE);
    }
    return t->disks[(t->top)--];
}

void print_tower(const Tower *t, char name)
{
    printf("Tower %c: ", name);
    for (int i = 0; i <= t->top; ++i)
    {
        printf("%d ", t->disks[i]);
    }
    printf("\n");
}

void print_state(const State *state)
{
    print_tower(&state->A, 'A');
    print_tower(&state->B, 'B');
    print_tower(&state->C, 'C');
    printf("\n");
}

void move_disk(State *state, char from_name, char to_name)
{
    Tower *from, *to;

    switch (from_name)
    {
    case 'A': from = &state->A; break;
    case 'B': from = &state->B; break;
    case 'C': from = &state->C; break;
    default:
        fprintf(stderr, "Invalid source tower\n");
        exit(EXIT_FAILURE);
    }

    switch (to_name)
    {
    case 'A': to = &state->A; break;
    case 'B': to = &state->B; break;
    case 'C': to = &state->C; break;
    default:
        fprintf(stderr, "Invalid destination tower\n");
        exit(EXIT_FAILURE);
    }

    int disk = pop(from);
    push(to, disk);
    printf("Move disk %d from %c to %c\n", disk, from_name, to_name);
}

void solve_hanoi(State *state, int n, char from, char to, char aux)
{
    if (n == 1)
    {
        move_disk(state, from, to);
        print_state(state);
        return;
    }

    solve_hanoi(state, n - 1, from, aux, to);
    move_disk(state, from, to);
    print_state(state);
    solve_hanoi(state, n - 1, aux, to, from);
}

int main()
{
    State state;
    int n = 3;

    init_state(&state, n);
    printf("Initial state:\n");
    print_state(&state);

    solve_hanoi(&state, n, 'A', 'C', 'B');

    return 0;
}