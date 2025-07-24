#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Count how many digits a number has
int count_digits(int n)
{
    int count = 0;

    if (n == 0)
        return 1; // special case: 0 has 1 digit

    if (n < 0)
        n = -n; // make it positive

    while (n > 0)
    {
        n /= 10;
        count++;
    }

    return count;
}

// Extract digits of a number into an array
int *get_digits(int n, int *len)
{
    if (n < 0)
        n = -n; // ignore sign

    if (n == 0)
    {
        *len = 1;
        int *digits = malloc(sizeof(int));
        digits[0] = 0;
        return digits;
    }

    int temp = n;
    *len = 0;

    // Count digits
    while (temp > 0)
    {
        temp /= 10;
        (*len)++;
    }

    // Allocate array
    int *digits = malloc(*len * sizeof(int));

    // Fill array in reverse order
    for (int i = *len - 1; i >= 0; i--)
    {
        digits[i] = n % 10;
        n /= 10;
    }

    return digits;
}

int main(int argc, char *argv[])
{
    // Word mappings for units, teens and tens
    char *units[] = {
        "zero", "one", "two", "three", "four",
        "five", "six", "seven", "eight", "nine"};

    char *eleven_nineteen[] = {
        "eleven", "twelve", "thirteen", "fourteen", "fifteen",
        "sixteen", "seventeen", "eighteen", "nineteen"};

    char *tens[] = {
        "ten", "twenty", "thirty", "forty", "fifty",
        "sixty", "seventy", "eighty", "ninety"};

    // Check argument count
    if (argc < 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    // Convert argument to integer
    int n = atoi(argv[1]);

    if (n < 0 || n > 9999)
    {
        printf("Number out of supported range (0–9999).\n");
        return 1;
    }

    // Special case: zero
    if (n == 0)
    {
        printf("zero\n");
        return 0;
    }

    // Extract digit positions
    int thousands = n / 1000;
    int hundreds = (n % 1000) / 100;
    int tens_digit = (n % 100) / 10;
    int units_digit = n % 10;

    // Print thousands
    if (thousands > 0)
    {
        printf("%s thousand", units[thousands]);
        if (hundreds > 0 || tens_digit > 0 || units_digit > 0)
            printf(" ");
    }

    // Print hundreds
    if (hundreds > 0)
    {
        printf("%s hundred", units[hundreds]);
        if (tens_digit > 0 || units_digit > 0)
            printf(" ");
    }

    // Special case: 11–19
    if (tens_digit == 1 && units_digit > 0)
    {
        printf("%s", eleven_nineteen[units_digit - 1]);
    }
    // Ten exactly
    else if (tens_digit == 1 && units_digit == 0)
    {
        printf("ten");
    }
    else
    {
        // Print tens (20, 30, ..., 90)
        if (tens_digit > 1)
        {
            printf("%s", tens[tens_digit - 1]);
            if (units_digit > 0)
                printf(" ");
        }

        // Print units
        if (units_digit > 0 && tens_digit != 1)
        {
            printf("%s", units[units_digit]);
        }
    }

    printf("\n");
    return 0;
}