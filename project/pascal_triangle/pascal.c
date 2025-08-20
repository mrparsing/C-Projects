#include <stdio.h>

long long factorial(int n)
{
    if (n == 0 || n == 1)
        return 1;
    return n * factorial(n - 1);
}

long binomial(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

int main()
{
    int row;

    do
    {
        printf("Enter row: ");
        scanf("%d", &row);
    } while (row < 1);

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < row - i; j++)
        {
            printf("  ");
        }

        for (int j = 0; j <= i; j++)
        {
            printf("%4ld", binomial(i, j));
        }

        printf("\n");
    }

    return 0;
}