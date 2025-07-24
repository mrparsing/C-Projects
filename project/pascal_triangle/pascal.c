#include <stdio.h>

// Recursive function to calculate factorial of n
long long factorial(int n)
{
    if (n == 0 || n == 1)
        return 1;
    return n * factorial(n - 1);
}

// Compute binomial coefficient "n choose k"
long binomial(int n, int k)
{
    return factorial(n) / (factorial(k) * factorial(n - k));
}

int main()
{
    int row;

    // Prompt the user for the number of rows in Pascal's Triangle (must be >= 1)
    do
    {
        printf("Enter row: ");
        scanf("%d", &row);
    } while (row < 1);

    // Print Pascal's Triangle up to the given number of rows
    for (int i = 0; i < row; i++)
    {
        // Print leading spaces for alignment (to center the triangle)
        for (int j = 0; j < row - i; j++)
        {
            printf("  ");
        }

        // Print binomial coefficients for the current row
        for (int j = 0; j <= i; j++)
        {
            printf("%4ld", binomial(i, j));
        }

        printf("\n");
    }

    return 0;
}