# Palindrome Checker in C

This C program checks whether a string passed via the command line is a **palindrome**—a string that reads the same forward and backward—while ignoring case differences.

## Compilation

```bash
chmod +x build.sh
./build.sh
```

## Run

```bash
./checker madam
String palindrome

./checker mAdaM
String palindrome

./checker Hello
String not palindrome
```

# How the Code Works
```c
char *string = argv[1];
int len = strlen(string);
```
The program retrieves the input string from the command-line argument.

It calculates the string’s length.

```c
for (int i = 0; i < len; i++) {
    if (tolower(string[i]) != tolower(string[len - i - 1])) {
        printf("String not palindrome\n");
        return 1;
    }
}
```
It iterates through the string, comparing each character at position i with the corresponding character from the end (len - i - 1).

Characters are converted to lowercase using tolower() to ensure case-insensitive comparison.

If any mismatch is found, it prints "String not palindrome" and exits.