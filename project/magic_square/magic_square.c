#include <stdio.h>
#include <stdlib.h>

#define N 3  // Size of the magic square (must be odd)

void print_square(int square[N][N])
{
    printf("Magic Square of size %d×%d:\n\n", N, N);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%2d ", square[i][j]);
        }
        printf("\n");
    }
}

// Generates an odd-sized magic square using the Siamese method
void generate_magic_square(int square[N][N])
{
    // Initialize all cells to 0
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            square[i][j] = 0;

    int num = 1;
    int i = 0;          // start from first row
    int j = N / 2;      // middle column

    while (num <= N * N)
    {
        square[i][j] = num; // place the current number

        int newi = (i - 1 + N) % N; // move up (with wrap-around)
        int newj = (j + 1) % N;     // move right (with wrap-around)

        if (square[newi][newj] != 0)
        {
            // if the cell is already filled, move down one row instead
            i = (i + 1) % N;
        }
        else
        {
            i = newi;
            j = newj;
        }

        num++;
    }
}

int main()
{
    int square[N][N];
    generate_magic_square(square);
    print_square(square);
    return 0;
}