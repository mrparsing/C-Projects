#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Check if a number was provided as argument
    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    long long number = atoi(argv[1]);  // Convert argument to integer
    int counter = 0;

    // Apply the Collatz sequence until the number reaches 1
    while (number != 1)
    {
        if (number % 2 == 0)
        {
            number /= 2;  // If even, divide by 2
        }
        else
        {
            number = 3 * number + 1;  // If odd, apply 3n + 1
        }

        counter++;

        // Print current step
        printf("Iteration: %d\nNumber: %lld\n--------------------\n", counter, number);
    }

    return 0;
}