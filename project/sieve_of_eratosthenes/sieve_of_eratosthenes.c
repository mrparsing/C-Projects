#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Ensure the user provided a number
    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    // Parse the number from command-line argument
    int n = atoi(argv[1]);

    // Print all numbers less than n that are not divisible by 2, 3, or 5
    for (int i = 2; i < n; i++)
    {
        // Special cases: 2, 3, and 5 are primes and should be printed
        if (i == 2 || i == 3 || i == 5)
        {
            printf("%d ", i);
            continue;
        }

        // Skip numbers divisible by 2, 3, or 5
        if (i % 2 == 0 || i % 3 == 0 || i % 5 == 0)
        {
            continue;
        }

        // Otherwise, print the number
        printf("%d ", i);
    }

    printf("\n");
    return 0;
}