# Tiny Regex/Grep

A super–small regular‑expression engine (supports `.` `*` `^` `$`) and a mini **grep** that scans stdin or files line‑by‑line. The core is \~40 lines split across three functions: `match`, `matchhere`, and `matchstar`.

> Original tutorial: [https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html](https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html)

---

## 1. What It Does

* **Regex engine:**

  * `.`  → any single character
  * `c*` → zero or more of character `c` (or any char if `c=='.'`)
  * `^`  → anchor at start of line
  * `$`  → anchor at end of line
* **Grep wrapper:** reads each input line and prints those that match.
* **No mallocs, no backtracking stacks** beyond C recursion. Pure C99.

---

## 2. Build & Run

### Compile

```bash
chmod +x build.sh
./build.sh
```

### Run on stdin

```bash
echo "hello world" | ./regex "^h.*d$"
```

### Run on files

```bash
./regex "ab*c" file1.txt file2.txt
```

If you pass only the pattern, it reads from **stdin**; with more args, each is treated as a file path.

Usage message (on wrong args):

```
Usage: ./regex <regex> [file...]
```

---

## 3. Code Walkthrough

Let’s go through the three key pieces. (Line numbers are conceptual — look for the function names.)

### 3.1 `match()` — entry point

```c
int match(char *regexp, char *text) {
    if (regexp[0] == '^')
        return matchhere(regexp + 1, text);
    do {
        if (matchhere(regexp, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}
```

**Idea:**

* If pattern starts with `^`, force match at beginning.
* Otherwise, slide the pattern across the text, testing each suffix (`text`, `text+1`, `text+2`, ...).

### 3.2 `matchhere()` — recursive core

```c
int matchhere(char *re, char *txt) {
    if (re[0] == '\0')            return 1;               // pattern exhausted → success
    if (re[1] == '*')              return matchstar(re[0], re+2, txt);
    if (re[0] == '$' && re[1]=='\0') return *txt=='\0';  // end anchor
    if (*txt!='\0' && (re[0]=='.' || re[0]==*txt))
        return matchhere(re+1, txt+1);                     // consume one char
    return 0;
}
```

**Cases:**

* Pattern empty → matched everything.
* Next token is `c*` → hand off to `matchstar`.
* End anchor `$` must coincide with end of text.
* Else, if current char matches (`.` or exact), consume both and recurse.

### 3.3 `matchstar()` — Kleene star handler

```c
int matchstar(int c, char *re, char *txt) {
    char *t;
    for (t = txt; *t!='\0' && (*t==c || c=='.'); t++);
    do {
        if (matchhere(re, t)) return 1;
    } while (t-- > txt);
    return 0;
}
```

**Greedy then backtrack:**

1. Walk forward over as many `c` as possible (or any char if `c=='.'`).
2. Then step back one by one, trying to match the rest of the pattern (`re`).

This “consume max then backtrack” is tiny but effective for simple regex.

### 3.4 The mini-grep `main()`

* Parses args: first is the regex, remaining are files (optional).
* Reads each line into a fixed buffer (`MAX_LINE`).
* Strips newline and calls `match(regex, line)`.
* Prints the line if it matches.

```c
if (argc == 2) {              // stdin mode
    while (fgets(line, MAX_LINE, stdin)) {
        line[strcspn(line, "\n")] = '\0';
        if (match(regex, line)) puts(line);
    }
} else {
    for (int i=2; i<argc; i++) {
        FILE *f = fopen(argv[i], "r");
        ...
    }
}
```

---

## 4. Supported Syntax (Quick Reference)

| Pattern piece | Meaning                              | Example               |
| ------------- | ------------------------------------ | --------------------- |
| `.`           | Match any single character           | `h.t` → `hat`, `hot`  |
| `c*`          | 0 or more of character `c` (literal) | `ab*c` → `ac`, `abbc` |
| `.*`          | 0 or more of **any** character       | `^foo.*bar$`          |
| `^`           | Anchor to start of string/line       | `^hello`              |
| `$`           | Anchor to end of string/line         | `world$`              |

> No escaping: `*` is special **only** when it follows a character. There’s no `+`, `?`, character classes, groups, alternation (`|`), etc.