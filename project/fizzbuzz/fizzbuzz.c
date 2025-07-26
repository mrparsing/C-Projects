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

    // Convert the input argument to an integer
    int number = atoi(argv[1]);

    // Iterate from 1 to the given number
    for (int i = 1; i <= number; i++)
    {
        if (i % 3 == 0 && i % 5 == 0)
            printf("FizzBuzz "); // Divisible by both 3 and 5
        else if (i % 3 == 0)
            printf("Fizz ");     // Divisible by 3 only
        else if (i % 5 == 0)
            printf("Buzz ");     // Divisible by 5 only
        else
            printf("%d ", i);    // Not divisible by 3 or 5
    }

    printf("\n");
    return 0;
}