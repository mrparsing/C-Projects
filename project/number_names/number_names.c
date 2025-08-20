#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int count_digits(int n)
{
    int count = 0;

    if (n == 0)
        return 1;

    if (n < 0)
        n = -n;

    while (n > 0)
    {
        n /= 10;
        count++;
    }

    return count;
}

int *get_digits(int n, int *len)
{
    if (n < 0)
        n = -n;

    if (n == 0)
    {
        *len = 1;
        int *digits = malloc(sizeof(int));
        digits[0] = 0;
        return digits;
    }

    int temp = n;
    *len = 0;

    while (temp > 0)
    {
        temp /= 10;
        (*len)++;
    }

    int *digits = malloc(*len * sizeof(int));

    for (int i = *len - 1; i >= 0; i--)
    {
        digits[i] = n % 10;
        n /= 10;
    }

    return digits;
}

int main(int argc, char *argv[])
{
    char *units[] = {
        "zero", "one", "two", "three", "four",
        "five", "six", "seven", "eight", "nine"};

    char *eleven_nineteen[] = {
        "eleven", "twelve", "thirteen", "fourteen", "fifteen",
        "sixteen", "seventeen", "eighteen", "nineteen"};

    char *tens[] = {
        "ten", "twenty", "thirty", "forty", "fifty",
        "sixty", "seventy", "eighty", "ninety"};

    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);

    if (n < 0 || n > 9999)
    {
        printf("Number out of supported range (0–9999).\n");
        return 1;
    }

    if (n == 0)
    {
        printf("zero\n");
        return 0;
    }

    int thousands = n / 1000;
    int hundreds = (n % 1000) / 100;
    int tens_digit = (n % 100) / 10;
    int units_digit = n % 10;

    if (thousands > 0)
    {
        printf("%s thousand", units[thousands]);
        if (hundreds > 0 || tens_digit > 0 || units_digit > 0)
            printf(" ");
    }

    if (hundreds > 0)
    {
        printf("%s hundred", units[hundreds]);
        if (tens_digit > 0 || units_digit > 0)
            printf(" ");
    }

    if (tens_digit == 1 && units_digit > 0)
    {
        printf("%s", eleven_nineteen[units_digit - 1]);
    }
    else if (tens_digit == 1 && units_digit == 0)
    {
        printf("ten");
    }
    else
    {
        if (tens_digit > 1)
        {
            printf("%s", tens[tens_digit - 1]);
            if (units_digit > 0)
                printf(" ");
        }

        if (units_digit > 0 && tens_digit != 1)
        {
            printf("%s", units[units_digit]);
        }
    }

    printf("\n");
    return 0;
}