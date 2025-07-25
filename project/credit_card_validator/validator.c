#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    char input[32];  // buffer for up to 31 characters + null terminator
    int digits[19];  // array to hold up to 19 digits (maximum card length)
    int len = 0;

    printf("Enter credit card number without spaces: ");
    if (scanf("%31s", input) != 1) {
        printf("Input error.\n");
        return 1;
    }

    // Extract digits from input and store them in the array
    for (int i = 0; input[i] != '\0' && len < 19; i++) {
        if (isdigit((unsigned char)input[i])) {
            digits[len++] = input[i] - '0';
        }
    }

    // Apply Luhn algorithm:
    // Starting from the second-to-last digit, moving left,
    // double every second digit. If result > 9, subtract 9.
    for (int i = len - 2; i >= 0; i -= 2) {
        int double_digit = digits[i] * 2;
        if (double_digit > 9) {
            double_digit -= 9;
        }
        digits[i] = double_digit;
    }

    // Sum all digits
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += digits[i];
    }

    // If total modulo 10 is 0, the number is valid
    if (sum % 10 == 0) {
        printf("VALID NUMBER!!\n");
    } else {
        printf("NOT VALID\n");
    }

    return 0;
}