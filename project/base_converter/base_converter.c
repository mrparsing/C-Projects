#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void print_as_binary(int n) {
    for (int i = (sizeof(int) * CHAR_BIT) - 1; i >= 0; i--) {
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

    char *end;
    long number = strtol(input_str, &end, base);

    if (*end != '\0') {
        fprintf(stderr, "Error: Invalid number '%s' for base %d.\n", input_str, base);
        return 1;
    }

    printf("Converting %s (base %d) to base %c: ", input_str, base, target_base_char);

    switch (target_base_char) {
        case 'D':
            printf("%ld\n", number);
            break;
        case 'H':
            printf("%lX\n", number);
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