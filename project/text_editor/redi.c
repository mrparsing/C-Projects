#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>

// Macro to interpret Ctrl key combinations (e.g., CTRL_KEY('q') becomes 17)
#define CTRL_KEY(k) ((k) & 0x1f)

// Version string of the editor
#define REDI_VERSION "0.0.1"

// Enumeration of special key codes
enum editorKey
{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

// Stores original terminal attributes so we can restore them on exit
struct termios orig_termios;

typedef struct erow
{
    int size;
    char *chars;
} erow;

// Global editor configuration
struct editorConfig
{
    int cx, cy;     // Cursor position (column, row)
    int screenrows; // Number of rows in the terminal
    int rowoff;
    int coloff;
    int screencols; // Number of columns in the terminal
    int numrows;
    erow *row;
    struct termios orig_termios; // Original terminal settings
};

struct editorConfig E;

/* ======== TERMINAL ======== */

// Print error message and exit, after clearing the screen
void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);  // Move cursor to top-left
    perror(s);
    exit(1);
}

// Restore original terminal settings (called automatically on exit)
void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

// Enable raw mode (turns off canonical mode, echoing, signals, etc.)
void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode); // Register cleanup function

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // Input flags
    raw.c_oflag &= ~(OPOST);                                  // Output flags
    raw.c_cflag |= (CS8);                                     // Character size
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);          // Local flags
    raw.c_cc[VMIN] = 0;                                       // Minimum bytes before read() returns
    raw.c_cc[VTIME] = 1;                                      // Timeout (tenths of a second)

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

// Reads a keypress and returns it, handling escape sequences for special keys
int editorReadKey(void)
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }

    if (c == '\x1b')
    { // Escape sequence
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1)
                    return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                    case '1':
                        return HOME_KEY;
                    case '3':
                        return DEL_KEY;
                    case '4':
                        return END_KEY;
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    case '7':
                        return HOME_KEY;
                    case '8':
                        return END_KEY;
                    }
                }
            }
            else
            {
                switch (seq[1])
                {
                case 'A':
                    return ARROW_UP;
                case 'B':
                    return ARROW_DOWN;
                case 'C':
                    return ARROW_RIGHT;
                case 'D':
                    return ARROW_LEFT;
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                }
            }
        }
        else if (seq[0] == 'O')
        {
            switch (seq[1])
            {
            case 'H':
                return HOME_KEY;
            case 'F':
                return END_KEY;
            }
        }

        return '\x1b';
    }
    else
    {
        return c;
    }
}

// Gets the current cursor position (used as fallback to determine window size)
int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;

    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
        return -1;

    return 0;
}

// Tries to get the terminal window size; if it fails, fallback using cursor position
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/* ======== ROW OPERATIONS ======== */

void editorAppendRow(char *s, size_t len)
{
    E.row = realloc(E.row, sizeof(erow) * (E.numrows + 1));
    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';
    E.numrows++;
}

/* ======== FILE I/O ======== */

void editorOpen(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        die("fopen");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fp)) != -1)
    {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
            linelen--;
        editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
}

/* ======== APPEND BUFFER ======== */

// Simple dynamic string buffer for efficient screen drawing
struct abuf
{
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

// Appends data to the buffer
void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);
    if (new == NULL)
        return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

// Frees the buffer
void abFree(struct abuf *ab)
{
    free(ab->b);
}

/* ======== INPUT ======== */

// Moves the cursor in response to arrow keys
void editorMoveCursor(int key)
{
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

    switch (key)
    {
    case ARROW_LEFT:
        if (E.cx != 0)
        {
            E.cx--;
        }
        else if (E.cy > 0)
        {
            E.cy--;
            E.cx = E.row[E.cy].size;
        }

        break;
    case ARROW_RIGHT:
        if (row && E.cx < row->size)
        {
            E.cx++;
        }
        else if (row && E.cx == row->size)
        {
            E.cy++;
            E.cx = 0;
        }
        break;
    case ARROW_UP:
        if (E.cy != 0)
            E.cy--;
        break;
    case ARROW_DOWN:
        if (E.cy < E.numrows)
            E.cy++;
        break;
    }
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen)
    {
        E.cx = rowlen;
    }
}

// Processes a key press and updates state or exits
void editorProcessKeypress(void)
{
    int c = editorReadKey();

    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
        write(STDOUT_FILENO, "\x1b[H", 3);  // Move cursor to top-left
        exit(0);
        break;

    case HOME_KEY:
        E.cx = 0;
        break;

    case END_KEY:
        E.cx = E.screencols - 1;
        break;

    case PAGE_UP:
    case PAGE_DOWN:
    {
        int times = E.screenrows;
        while (times--)
            editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        break;
    }

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
        editorMoveCursor(c);
        break;
    }
}

/* ======== OUTPUT ======== */

void editorScroll()
{
    if (E.cy < E.rowoff)
    {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows)
    {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.cx < E.coloff)
    {
        E.coloff = E.cx;
    }
    if (E.cx >= E.coloff + E.screencols)
    {
        E.coloff = E.cx - E.screencols + 1;
    }
}

// Draws the content of each row on the screen (currently just ~ and a welcome message)
void editorDrawRows(struct abuf *ab)
{
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        int filerow = y + E.rowoff;
        if (filerow >= E.numrows)
        {
            if (E.numrows == 0 && y == E.screenrows / 3)
            {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome),
                                          "Redi editor -- version %s", REDI_VERSION);
                if (welcomelen > E.screencols)
                    welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding)
                {
                    abAppend(ab, "~", 1);
                    padding--;
                }
                while (padding--)
                    abAppend(ab, " ", 1);
                abAppend(ab, welcome, welcomelen);
            }
            else
            {
                abAppend(ab, "~", 1);
            }
        }
        else
        {
            int len = E.row[filerow].size - E.coloff;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;
            abAppend(ab, &E.row[filerow].chars[E.coloff], len);
        }
        abAppend(ab, "\x1b[K", 3);
        if (y < E.screenrows - 1)
        {
            abAppend(ab, "\r\n", 2);
        }
    }
}

// Refreshes the entire screen
void editorRefreshScreen(void)
{
    editorScroll();
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // Hide cursor
    abAppend(&ab, "\x1b[H", 3);    // Move cursor to top-left

    editorDrawRows(&ab); // Draw rows

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1,
             (E.cx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6); // Show cursor

    write(STDOUT_FILENO, ab.b, ab.len); // Write to terminal
    abFree(&ab);
}

/* ======== INIT ======== */

// Initializes the editor (sets cursor position and gets terminal size)
void initEditor(void)
{
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;

    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}

/* ======== MAIN ======== */

int main(int argc, char *argv[])
{
    enableRawMode(); // Enter raw mode
    initEditor();    // Set up screen size and state
    if (argc >= 2)
    {
        editorOpen(argv[1]);
    }

    while (1)
    {
        editorRefreshScreen();   // Redraw screen
        editorProcessKeypress(); // Wait and handle keypress
    }

    return 0;
}
