#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

// === PROTOTYPES ===
int match(char *regexp, char *text);
int matchhere(char *regexp, char *text);
int matchstar(int c, char *regexp, char *text);

// === Top-level match function ===
// Supports optional ^ anchor. Tries to match the regex anywhere in text.
int match(char *regexp, char *text) {
    if (regexp[0] == '^')
        return matchhere(regexp + 1, text);  // anchored at beginning
    do {
        if (matchhere(regexp, text))
            return 1;  // match found
    } while (*text++ != '\0');
    return 0;
}

// === Match from here (core of the engine) ===
int matchhere(char *regexp, char *text) {
    if (regexp[0] == '\0')
        return 1;  // end of regex, success
    if (regexp[1] == '*')
        return matchstar(regexp[0], regexp + 2, text);  // handle x*
    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\0';  // end of line anchor
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
        return matchhere(regexp + 1, text + 1);  // match one char
    return 0;  // no match
}

// === Handle c* (zero or more of char c) ===
int matchstar(int c, char *regexp, char *text) {
    char *t;
    // Consume as many c as possible (greedy)
    for (t = text; *t != '\0' && (*t == c || c == '.'); t++);
    // Try matching regex from the end of that run, backtracking
    do {
        if (matchhere(regexp, t))
            return 1;
    } while (t-- > text);
    return 0;
}

// === Main function: mini grep ===
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <regex> [file...]\n", argv[0]);
        return 1;
    }

    char line[MAX_LINE];
    char *regex = argv[1];

    if (argc == 2) {
        // Read from stdin if no files are provided
        while (fgets(line, MAX_LINE, stdin)) {
            line[strcspn(line, "\n")] = '\0';  // remove trailing newline
            if (match(regex, line))
                puts(line);  // print matching line
        }
    } else {
        // Read from files
        for (int i = 2; i < argc; i++) {
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                perror(argv[i]);
                continue;
            }

            while (fgets(line, MAX_LINE, f)) {
                line[strcspn(line, "\n")] = '\0';  // remove trailing newline
                if (match(regex, line))
                    puts(line);
            }

            fclose(f);
        }
    }

    return 0;
}