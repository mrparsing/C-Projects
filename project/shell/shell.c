#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

int main()
{
    initscr();
    noecho();
    raw();

    bool QUIT = false;
    int ch;
    String command = {0};
    Strings command_his = {0};
    size_t line = 0;

    while (!QUIT)
    {
        move(line, 0);
        clrtoeol(); // pulisce tutta la riga prima di scrivere
        mvprintw(line, 0, "> ");
        mvprintw(line, 2, "%.*s", (int)command.count, command.data);
        ch = getch();

        switch (ch)
        {
        case ctrl('q'):
            QUIT = true;
            break;
        case 10: // ENTER
            line++;
            strings_append(&command_his, &command);
            if (command.count > 0)
            {
                mvprintw(line, 0, "sh: %.*s: command not found", (int)command.count, command.data);
                line++;
            }
            string_free(&command);
            break;
        default:
            string_append(&command, ch);
            break;
        }
    }

    refresh();
    endwin();

    // Clean up
    for (size_t i = 0; i < command_his.count; i++)
        string_free(&command_his.data[i]);
    free(command_his.data);

    return 0;
}