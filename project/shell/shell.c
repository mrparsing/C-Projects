#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DOWN_ARROW 258
#define UP_ARROW 259
#define LEFT_ARROW 260
#define RIGHT_ARROW 261

#define ctrl(x) ((x) & 0x1f)

#define DATA_START_CAPACITY 128

#define ASSERT(cond, ...)                                                     \
    do                                                                        \
    {                                                                         \
        if (!(cond))                                                          \
        {                                                                     \
            endwin();                                                         \
            fprintf(stderr, "%s:%d: ASSERTION FAILED: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                                     \
            fprintf(stderr, "\n");                                            \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

#define DA_APPEND(da, item)                                                                  \
    do                                                                                       \
    {                                                                                        \
        if ((da)->count >= (da)->capacity)                                                   \
        {                                                                                    \
            (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity * 2; \
            void *new = calloc((da)->capacity, sizeof(char));                                \
            ASSERT(new, "out of memory");                                                    \
            memcpy(new, (da)->data, (da)->count);                                            \
            free((da)->data);                                                                \
            (da)->data = new;                                                                \
        }                                                                                    \
        ((char *)(da)->data)[(da)->count++] = (item);                                        \
    } while (0)

typedef struct
{
    char *data;
    size_t count;
    size_t capacity;
} String;

typedef struct
{
    String *data;
    size_t count;
    size_t capacity;
} Strings;

void string_append(String *s, char c)
{
    if (s->count >= s->capacity)
    {
        s->capacity = s->capacity == 0 ? DATA_START_CAPACITY : s->capacity * 2;
        char *new = calloc(s->capacity, sizeof(char));
        ASSERT(new, "no memory");
        memcpy(new, s->data, s->count);
        free(s->data);
        s->data = new;
    }
    s->data[s->count++] = c;
}

void strings_append(Strings *arr, String *s)
{
    if (arr->count >= arr->capacity)
    {
        arr->capacity = arr->capacity == 0 ? 16 : arr->capacity * 2;
        String *new = calloc(arr->capacity, sizeof(String));
        ASSERT(new, "no memory");
        memcpy(new, arr->data, arr->count * sizeof(String));
        free(arr->data);
        arr->data = new;
    }

    // Deep copy of the string
    String copy = {0};
    copy.data = malloc(s->count);
    ASSERT(copy.data, "no memory");
    memcpy(copy.data, s->data, s->count);
    copy.count = s->count;
    copy.capacity = s->count;
    arr->data[arr->count++] = copy;
}

void string_free(String *s)
{
    free(s->data);
    s->data = NULL;
    s->count = s->capacity = 0;
}

void string_insert(String *s, size_t pos, char c)
{
    if (s->count >= s->capacity)
    {
        s->capacity = s->capacity == 0 ? DATA_START_CAPACITY : s->capacity * 2;
        char *new = calloc(s->capacity, sizeof(char));
        ASSERT(new, "no memory");
        memcpy(new, s->data, pos);
        memcpy(new + pos + 1, s->data + pos, s->count - pos);
        free(s->data);
        s->data = new;
    }
    else
    {
        memmove(s->data + pos + 1, s->data + pos, s->count - pos);
    }

    s->data[pos] = c;
    s->count++;
}

char *string_to_cstr(String *s)
{
    char *res = malloc(s->count + 1);
    ASSERT(res, "no memory");
    memcpy(res, s->data, s->count);
    res[s->count] = '\0';
    return res;
}

char **tokenize(const char *cmd, int *argc_out)
{
    int capacity = 8;
    int argc = 0;
    char **argv = malloc(capacity * sizeof(char *));
    ASSERT(argv, "no memory");

    char *cmd_copy = strdup(cmd);
    ASSERT(cmd_copy, "no memory");
    char *token = strtok(cmd_copy, " \t\n");
    while (token)
    {
        if (argc >= capacity)
        {
            capacity *= 2;
            argv = realloc(argv, capacity * sizeof(char *));
            ASSERT(argv, "no memory");
        }
        argv[argc++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }

    argv[argc] = NULL;
    *argc_out = argc;
    free(cmd_copy);
    return argv;
}

int main()
{
    initscr();
    noecho();
    raw();
    keypad(stdscr, TRUE);

    bool QUIT = false;
    int ch;
    String command = {0};
    Strings command_history = {0};
    size_t line = 0;
    size_t current_command = 0;
    size_t command_max = 0;
    size_t cursor = 0;                   // current cursor position in the line
    ssize_t current_command_index = -1;  // -1 = no command selected
    bool editing_history = false;

    while (!QUIT)
    {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        if (line >= rows)
        {
            scrl(10);
            line = rows - 19;
        }

        move(line, 0);
        clrtoeol(); // clear the entire line before writing
        mvprintw(line, 0, "> ");
        mvprintw(line, 2, "%.*s", (int)command.count, command.data);
        move(line, 2 + cursor);
        ch = getch();

        switch (ch)
        {
        case ctrl('q'):
            QUIT = true;
            break;
        case 10: // ENTER
            // Finalize the command line on screen and go to next line
            mvprintw(line, 0, "> %.*s", (int)command.count, command.data);
            line++;
            move(line, 0); // Move cursor to the new line

            strings_append(&command_history, &command);

            if (command.count > 0)
            {
                char *cmd_cstr = string_to_cstr(&command);
                int argc;
                char **argv = tokenize(cmd_cstr, &argc);

                // ✅ 1. Create a pipe
                int pipefd[2];
                if (pipe(pipefd) == -1)
                {
                    perror("pipe");
                    break;
                }

                pid_t pid = fork();
                if (pid == -1)
                {
                    perror("fork");
                    break;
                }

                if (pid == 0)
                {
                    // --- Child Process ---
                    // We don't need endwin() here

                    // ✅ 2. Redirect stdout to the pipe
                    close(pipefd[0]);                // Child doesn't read from pipe
                    dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout
                    dup2(pipefd[1], STDERR_FILENO);  // Also redirect stderr (optional)
                    close(pipefd[1]);

                    execvp(argv[0], argv);
                    fprintf(stderr, "sh: %s: command not found\n", argv[0]);
                    exit(127);
                }
                else
                {
                    // --- Parent Process ---
                    close(pipefd[1]); // Parent doesn't write to pipe

                    // ✅ 3. Read output from pipe and print it with ncurses
                    char buffer[256];
                    ssize_t nbytes;
                    while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
                    {
                        buffer[nbytes] = '\0';
                        // Print buffer character by character to properly handle newlines
                        for (ssize_t i = 0; i < nbytes; ++i)
                        {
                            if (buffer[i] == '\n')
                            {
                                clrtoeol(); // Clear rest of line
                                line++;
                                move(line, 0);
                            }
                            else
                            {
                                printw("%c", buffer[i]);
                            }
                        }
                    }

                    close(pipefd[0]);
                    waitpid(pid, NULL, 0);
                }

                // Cleanup
                for (int i = 0; i < argc; i++)
                    free(argv[i]);
                free(argv);
                free(cmd_cstr);
            }

            // Reset for next command
            string_free(&command);
            cursor = 0;
            current_command_index = -1;
            editing_history = false;

            // Ensure line is not empty for next prompt
            clrtoeol();
            break;
        case UP_ARROW:
            if (command_history.count > 0 && current_command_index + 1 < (ssize_t)command_history.count)
            {
                current_command_index++;
                string_free(&command);
                String *src = &command_history.data[command_history.count - 1 - current_command_index];
                command.data = malloc(src->count);
                ASSERT(command.data, "no memory");
                memcpy(command.data, src->data, src->count);
                command.count = src->count;
                command.capacity = src->count;
                cursor = command.count;
                editing_history = true;
            }
            break;

        case DOWN_ARROW:
            if (current_command_index > 0)
            {
                current_command_index--;
                string_free(&command);
                String *src = &command_history.data[command_history.count - 1 - current_command_index];
                command.data = malloc(src->count);
                ASSERT(command.data, "no memory");
                memcpy(command.data, src->data, src->count);
                command.count = src->count;
                command.capacity = src->count;
                cursor = command.count;
                editing_history = true;
            }
            else if (current_command_index == 0)
            {
                current_command_index = -1;
                string_free(&command);
                command.data = NULL;
                command.count = command.capacity = 0;
            }
            break;
        case LEFT_ARROW:
            if (cursor > 0)
                cursor--;
            break;
        case RIGHT_ARROW:
            if (cursor < command.count)
                cursor++;
            break;
        case 127: // BACKSPACE (ASCII)
        case KEY_BACKSPACE:
            if (cursor > 0)
            {
                memmove(command.data + cursor - 1, command.data + cursor, command.count - cursor);
                command.count--;
                cursor--;
            }
            break;
        default:
            if (editing_history)
            {
                // About to modify a command from history → make a copy to avoid destructive changes
                String copy = {0};
                copy.data = malloc(command.count);
                ASSERT(copy.data, "no memory");
                memcpy(copy.data, command.data, command.count);
                copy.count = command.count;
                copy.capacity = command.count;

                string_free(&command);
                command = copy;

                current_command_index = -1;
                editing_history = false;
            }
            string_insert(&command, cursor, ch);
            cursor++;
            break;
        }
        refresh();
    }

    refresh();
    endwin();

    // Clean up
    for (size_t i = 0; i < command_history.count; i++)
        string_free(&command_history.data[i]);
    free(command_history.data);

    return 0;
}