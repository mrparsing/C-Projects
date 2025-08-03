# Word and Character Counter

This is a simple C program that reads a text file and counts:
- The number of **words**
- The number of **characters**, excluding whitespace (spaces, tabs, newlines)

## Features

- Reads input file name from user
- Parses the file character by character
- Handles multiple spaces, tabs, and newlines correctly
- Detects word boundaries based on whitespace
- Prevents buffer overflows when reading the file name

## How It Works

1. **Prompt for file name**  
   The program asks the user to input the file name, using `scanf` with size-limited input to avoid overflow.

2. **Open the file**  
   It attempts to open the specified file in read mode. If the file doesn't exist or can't be opened, it shows an error.

3. **Count words and characters**  
   It reads the file one character at a time:
   - If the character is **not whitespace** (`isspace(x) == false`), it's counted as a character.
   - If we're **not already inside a word**, a new word is detected.
   - If a whitespace character is encountered, it marks the end of a word.

4. **Display results**  
   Once the file has been fully processed, it prints:
   - Total characters (excluding whitespace)
   - Total word count