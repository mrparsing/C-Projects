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
            ASSERT(new, "outta ram");                                                        \
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
        ASSERT(new, "no mem");
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
        ASSERT(new, "no mem");
        memcpy(new, arr->data, arr->count * sizeof(String));
        free(arr->data);
        arr->data = new;
    }

    // Deep copy della stringa
    String copy = {0};
    copy.data = malloc(s->count);
    ASSERT(copy.data, "no mem");
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
        ASSERT(new, "no mem");
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
    ASSERT(res, "no mem");
    memcpy(res, s->data, s->count);
    res[s->count] = '\0';
    return res;
}

char **tokenize(const char *cmd, int *argc_out)
{
    int capacity = 8;
    int argc = 0;
    char **argv = malloc(capacity * sizeof(char *));
    ASSERT(argv, "no mem");

    char *cmd_copy = strdup(cmd);
    ASSERT(cmd_copy, "no mem");
    char *token = strtok(cmd_copy, " \t\n");
    while (token)
    {
        if (argc >= capacity)
        {
            capacity *= 2;
            argv = realloc(argv, capacity * sizeof(char *));
            ASSERT(argv, "no mem");
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
    Strings command_his = {0};
    size_t line = 0;
    size_t current_command = 0;
    size_t command_max = 0;
    size_t cursor = 0;                  // posizione attuale del cursore nella riga
    ssize_t current_command_index = -1; // -1 = nessun comando selezionato
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
        clrtoeol(); // pulisce tutta la riga prima di scrivere
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
            // Finalizza la riga del comando sullo schermo e vai a capo
            mvprintw(line, 0, "> %.*s", (int)command.count, command.data);
            line++;
            move(line, 0); // Sposta il cursore sulla nuova linea

            strings_append(&command_his, &command);

            if (command.count > 0)
            {
                char *cmd_cstr = string_to_cstr(&command);
                int argc;
                char **argv = tokenize(cmd_cstr, &argc);

                // ✅ 1. Crea una pipe
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
                    // --- Processo Figlio ---
                    // NON abbiamo bisogno di endwin() qui

                    // ✅ 2. Redireziona stdout alla pipe
                    close(pipefd[0]);               // Il figlio non legge dalla pipe
                    dup2(pipefd[1], STDOUT_FILENO); // Redireziona stdout
                    dup2(pipefd[1], STDERR_FILENO); // Redireziona anche stderr (opzionale)
                    close(pipefd[1]);

                    execvp(argv[0], argv);
                    fprintf(stderr, "sh: %s: command not found\n", argv[0]);
                    exit(127);
                }
                else
                {
                    // --- Processo Padre ---
                    close(pipefd[1]); // Il padre non scrive sulla pipe

                    // ✅ 3. Leggi l'output dalla pipe e stampalo con ncurses
                    char buffer[256];
                    ssize_t nbytes;
                    while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
                    {
                        buffer[nbytes] = '\0';
                        // Stampa il buffer carattere per carattere per gestire correttamente i newline
                        for (ssize_t i = 0; i < nbytes; ++i)
                        {
                            if (buffer[i] == '\n')
                            {
                                clrtoeol(); // Pulisce il resto della riga
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

            // Resetta per il prossimo comando
            string_free(&command);
            cursor = 0;
            current_command_index = -1;
            editing_history = false;

            // Assicura che la linea non sia vuota per il prossimo prompt
            clrtoeol();
            break;
        case UP_ARROW:
            if (command_his.count > 0 && current_command_index + 1 < (ssize_t)command_his.count)
            {
                current_command_index++;
                string_free(&command);
                String *src = &command_his.data[command_his.count - 1 - current_command_index];
                command.data = malloc(src->count);
                ASSERT(command.data, "no mem");
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
                String *src = &command_his.data[command_his.count - 1 - current_command_index];
                command.data = malloc(src->count);
                ASSERT(command.data, "no mem");
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
                // Stai per modificare un comando preso dalla cronologia → fai una copia per evitare modifiche distruttive
                String copy = {0};
                copy.data = malloc(command.count);
                ASSERT(copy.data, "no mem");
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
    for (size_t i = 0; i < command_his.count; i++)
        string_free(&command_his.data[i]);
    free(command_his.data);

    return 0;
}