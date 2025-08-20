#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define R 200
#define NUM_POINTS 100000

struct Point {
    double x;
    double y;
};

void generate_circle(SDL_Surface *surface) {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            double dx = i - WIDTH / 2;
            double dy = j - HEIGHT / 2;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance >= R - 1 && distance <= R + 1) {
                SDL_Rect pixel = {i, j, 1, 1};
                SDL_FillRect(surface, &pixel, COLOR_WHITE);
            }
        }
    }
}

void print_point(SDL_Surface *surface, double dx, double dy) {
    SDL_Rect pixel = {(int)dx, (int)dy, 1, 1};
    SDL_FillRect(surface, &pixel, COLOR_WHITE);
}

void generate_random_point(SDL_Surface *surface) {
    int counter = 0;

    for (int i = 0; i < NUM_POINTS; i++) {
        double x = ((double)rand() / RAND_MAX) * 2 * R - R;
        double y = ((double)rand() / RAND_MAX) * 2 * R - R;

        double distance = sqrt(x * x + y * y);
        if (distance <= R)
            counter++;

        double dx = x + WIDTH / 2;
        double dy = y + HEIGHT / 2;

        print_point(surface, dx, dy);
    }

    double pi_estimation = 4.0 * counter / NUM_POINTS;
    printf("PI estimation: %f\n", pi_estimation);
}

int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "PI Approximation",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    generate_circle(surface);
    generate_random_point(surface);

    SDL_UpdateWindowSurface(window);

    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }
        SDL_Delay(10);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}