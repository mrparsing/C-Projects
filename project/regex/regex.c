#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

int match(char *regexp, char *text);
int matchhere(char *regexp, char *text);
int matchstar(int c, char *regexp, char *text);

int match(char *regexp, char *text) {
    if (regexp[0] == '^')
        return matchhere(regexp + 1, text);
    do {
        if (matchhere(regexp, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}

int matchhere(char *regexp, char *text) {
    if (regexp[0] == '\0')
        return 1;
    if (regexp[1] == '*')
        return matchstar(regexp[0], regexp + 2, text);
    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\0';
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
        return matchhere(regexp + 1, text + 1);
    return 0;
}

int matchstar(int c, char *regexp, char *text) {
    char *t;
    for (t = text; *t != '\0' && (*t == c || c == '.'); t++);
    do {
        if (matchhere(regexp, t))
            return 1;
    } while (t-- > text);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <regex> [file...]\n", argv[0]);
        return 1;
    }

    char line[MAX_LINE];
    char *regex = argv[1];

    if (argc == 2) {
        while (fgets(line, MAX_LINE, stdin)) {
            line[strcspn(line, "\n")] = '\0';
            if (match(regex, line))
                puts(line);
        }
    } else {
        for (int i = 2; i < argc; i++) {
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                perror(argv[i]);
                continue;
            }

            while (fgets(line, MAX_LINE, f)) {
                line[strcspn(line, "\n")] = '\0'; 
                if (match(regex, line))
                    puts(line);
            }

            fclose(f);
        }
    }

    return 0;
}