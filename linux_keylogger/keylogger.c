#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <glob.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#include "keymap.h"

static int fd = -1;

/* Scorciatoia: prima tastiera trovata in /dev/input/by-path */
static const char *find_keyboard_by_path(void)
{
    static char result[PATH_MAX] = {0};
    glob_t g;

    if (glob("/dev/input/by-path/*-event-kbd", 0, NULL, &g) == 0 &&
        g.gl_pathc > 0)
    {
        strncpy(result, g.gl_pathv[0], sizeof(result) - 1);
        globfree(&g);
        return result; /* symlink stabile alla tastiera */
    }
    globfree(&g);
    return NULL;
}

static void handle_sigint(int sig)
{
    (void)sig;
    if (fd >= 0)
        close(fd);
    putchar('\n');
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    const char *device = find_keyboard_by_path();
    if (!device)
    {
        fprintf(stderr,
                "Nessuna tastiera trovata in /dev/input/by-path/*-event-kbd\n");
        return EXIT_FAILURE;
    }

    printf("Uso device: %s\n", device);

    fd = open(device, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Gestisco Ctrl-C */
    struct sigaction sa = {.sa_handler = handle_sigint, .sa_flags = SA_RESTART};
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    struct input_event ev;
    while (1)
    {
        ssize_t bytes = read(fd, &ev, sizeof ev);
        if (bytes < 0)
        {
            if (errno == EINTR)
                continue; /* interrotto da segnale */
            perror("read");
            break;
        }
        if (bytes != sizeof ev)
        {
            fprintf(stderr, "Short read\n");
            break;
        }

        if (ev.type == EV_KEY && ev.value == 1)
        { /* key press */
            if (ev.code < KEY_CNT && key_code_names[ev.code])
                printf("%s ", key_code_names[ev.code]);
            else
                printf("KEY_%u ", ev.code);
            fflush(stdout);
        }
    }

    close(fd);
    return EXIT_FAILURE;
}