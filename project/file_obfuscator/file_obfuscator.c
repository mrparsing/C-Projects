#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int transform(int c, const char *mode, int key) {
    if (strcmp(mode, "not") == 0) {
        return ~c & 0xFF;
    } else if (strcmp(mode, "xor") == 0) {
        return c ^ key;
    } else {
        return c;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s not input.txt output.txt\n", argv[0]);
        fprintf(stderr, "  %s xor <key> input.txt output.txt\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    int key = 0;
    const char *input_file;
    const char *output_file;

    if (strcmp(mode, "not") == 0 && argc == 4) {
        input_file = argv[2];
        output_file = argv[3];
    } else if (strcmp(mode, "xor") == 0 && argc == 5) {
        key = atoi(argv[2]);
        input_file = argv[3];
        output_file = argv[4];
    } else {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    FILE *in = fopen(input_file, "rb");
    if (!in) {
        perror("Error opening input");
        return 1;
    }

    FILE *out = fopen(output_file, "wb");
    if (!out) {
        perror("Error opening output");
        fclose(in);
        return 1;
    }

    int c;
    while ((c = fgetc(in)) != EOF) {
        fputc(transform(c, mode, key), out);
    }

    fclose(in);
    fclose(out);
    return 0;
}