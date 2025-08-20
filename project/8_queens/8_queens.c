#include <stdio.h>
#include <stdlib.h>

#define N 8

int solutions = 0;

void print_chessboard(int queens[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (queens[i] == j)
                printf("Q ");
            else
                printf(". ");
        }
        printf("\n");
    }
    printf("\n");
}

int is_safe(int queens[N], int row, int col) {
    for (int i = 0; i < row; i++) {
        if (queens[i] == col || abs(queens[i] - col) == row - i)
            return 0;
    }
    return 1;
}

// Recursive backtracking solver
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

int main() {
    int queens[N];
    solve(queens, 0);
    printf("Found %d solutions.\n", solutions);
    return 0;
}