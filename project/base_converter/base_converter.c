#include <stdio.h>
#include <stdlib.h> // For strtol
#include <string.h> // For strcmp
#include <limits.h> // For CHAR_BIT

// A corrected function to print an integer's binary representation
void print_as_binary(int n) {
    // A 32-bit integer has 32 bits. sizeof(int) * CHAR_BIT gives the exact number of bits.
    // We start from the most significant bit (MSB).
    for (int i = (sizeof(int) * CHAR_BIT) - 1; i >= 0; i--) {
        // Shift the bit we want to the rightmost position and check if it's 1
        int bit = (n >> i) & 1;
        printf("%d", bit);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: %s <number> <source_base> <target_base>\n", argv[0]);
        printf("Bases: D (Decimal), B (Binary), H (Hexadecimal)\n");
        return 1;
    }

    char *input_str = argv[1];
    char source_base_char = argv[2][0];
    char target_base_char = argv[3][0];

    int base = 0;
    // Determine the source base for strtol
    switch (source_base_char) {
        case 'D':
            base = 10;
            break;
        case 'B':
            base = 2;
            break;
        case 'H':
            base = 16;
            break;
        default:
            fprintf(stderr, "Error: Invalid source base '%c'.\n", source_base_char);
            return 1;
    }

    // Use strtol to correctly parse the input string from any base
    char *end;
    long number = strtol(input_str, &end, base);

    // Check if strtol failed to parse the number
    if (*end != '\0') {
        fprintf(stderr, "Error: Invalid number '%s' for base %d.\n", input_str, base);
        return 1;
    }

    printf("Converting %s (base %d) to base %c: ", input_str, base, target_base_char);

    // Now, print the number in the target format
    switch (target_base_char) {
        case 'D':
            printf("%ld\n", number); // %ld for long
            break;
        case 'H':
            printf("%lX\n", number); // %lX for long hexadecimal
            break;
        case 'B':
            print_as_binary(number);
            printf("\n");
            break;
        default:
            fprintf(stderr, "Error: Invalid target base '%c'.\n", target_base_char);
            return 1;
    }

    return 0;
}