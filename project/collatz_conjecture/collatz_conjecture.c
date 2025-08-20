#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    long long number = atoi(argv[1]);
    int counter = 0;

    while (number != 1)
    {
        if (number % 2 == 0)
        {
            number /= 2; 
        }
        else
        {
            number = 3 * number + 1;
        }

        counter++;

        printf("Iteration: %d\nNumber: %lld\n--------------------\n", counter, number);
    }

    return 0;
}