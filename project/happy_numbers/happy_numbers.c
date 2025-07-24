#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Returns the sum of the squares of the digits of a number
int sum_squared_digits(int n) {
    int sum = 0;

    while (n > 0) {
        int digit = n % 10;
        sum += digit * digit;
        n /= 10;
    }

    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // If the user didn't provide a number, show usage
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    // Parse the input number
    int number = atoi(argv[1]);

    // Initialize two pointers: slow and fast
    int slow = number;
    int fast = sum_squared_digits(number);

    // Floyd's cycle detection algorithm ("tortoise and hare")
    // We stop if we reach 1 (happy number) or if slow == fast (cycle)
    while (fast != 1 && slow != fast) {
        slow = sum_squared_digits(slow);                    // move one step
        fast = sum_squared_digits(sum_squared_digits(fast)); // move two steps
    }

    if (fast == 1) {
        // If we reach 1, it's a happy number
        printf("CONGRATULATION, %d IS A HAPPY NUMBER!\n", number);
    } else {
        // Otherwise, we found a cycle that does not include 1
        printf("SORRY, %d IS NOT A HAPPY NUMBER.\n", number);
    }

    return 0;
}