#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    char input[32];
    int digits[19];
    int len = 0;

    printf("Enter credit card number without spaces: ");
    if (scanf("%31s", input) != 1) {
        printf("Input error.\n");
        return 1;
    }

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

    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += digits[i];
    }

    if (sum % 10 == 0) {
        printf("VALID NUMBER!!\n");
    } else {
        printf("NOT VALID\n");
    }

    return 0;
}