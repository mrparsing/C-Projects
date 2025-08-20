#include <stdio.h>
#include <stdlib.h>

#define N 3

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

void generate_magic_square(int square[N][N])
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            square[i][j] = 0;

    int num = 1;
    int i = 0;
    int j = N / 2;

    while (num <= N * N)
    {
        square[i][j] = num;

        int newi = (i - 1 + N) % N;
        int newj = (j + 1) % N;

        if (square[newi][newj] != 0)
        {
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