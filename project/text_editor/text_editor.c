#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

// Global variable to store the original terminal settings
struct termios orig_termios;

// Restores the original terminal settings when the program exits
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Enables "raw" mode by disabling input echoing
void enableRawMode()
{
    // Get and save the current terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);

    // Register a cleanup function to restore terminal settings on exit
    atexit(disableRawMode);

    // Make a copy of the original settings to modify
    struct termios raw = orig_termios;

    // Turn off the ECHO flag so typed characters are not printed
    raw.c_lflag &= ~(ECHO);

    // Apply the modified settings to the terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
    enableRawMode(); // Activate raw mode

    char c;
    // Read and discard characters one at a time until 'q' is pressed
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
        ;

    return 0;
}