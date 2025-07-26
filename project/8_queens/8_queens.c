#include <stdio.h>
#include <stdlib.h>

#define N 8 // Size of the chessboard (8x8)

// Counter for total solutions
int solutions = 0;

// Print the current board configuration
void print_chessboard(int queens[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (queens[i] == j)
                printf("Q "); // Queen at this position
            else
                printf(". "); // Empty square
        }
        printf("\n");
    }
    printf("\n");
}

// Check if placing a queen at (row, col) is safe
int is_safe(int queens[N], int row, int col) {
    for (int i = 0; i < row; i++) {
        // Check same column or same diagonal
        if (queens[i] == col || abs(queens[i] - col) == row - i)
            return 0;
    }
    return 1;
}

// Recursive backtracking solver
void solve(int queens[N], int row) {
    if (row == N) {
        solutions++;
        print_chessboard(queens); // Optional: remove if only count is needed
        return;
    }

    for (int col = 0; col < N; col++) {
        if (is_safe(queens, row, col)) {
            queens[row] = col;
            solve(queens, row + 1); // Try placing queen in next row
            // No need to undo: value will be overwritten in the next iteration
        }
    }
}

int main() {
    int queens[N]; // queens[i] = column where the queen is placed in row i
    solve(queens, 0);
    printf("Found %d solutions.\n", solutions);
    return 0;
}