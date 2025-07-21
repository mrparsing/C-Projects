#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define MAX_POINTS 200  // Maximum number of stars on screen

// Represents a 2D point (used for stars)
struct Point {
    int x, y;
};

// Move point away from the center proportionally to its distance (creates warp effect)
void calculate_rect(struct Point *p, double dt) {
    struct Point center = {WIDTH / 2, HEIGHT / 2};
    struct Point v = {p->x - center.x, p->y - center.y};  // Direction vector from center
    p->x += dt * v.x;
    p->y += dt * v.y;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Starfield",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    if (!surface) {
        fprintf(stderr, "SDL_GetWindowSurface error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event e;

    srand(time(NULL));  // Seed for random star positions
    Uint32 prev_ticks = SDL_GetTicks();

    struct Point points[MAX_POINTS];  // Array of stars
    int index = 0;

    while (running) {
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0;  // Delta time in seconds
        prev_ticks = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
        }

        // Generate a new random point
        int r1 = rand() % WIDTH;
        int r2 = rand() % HEIGHT;

        // Add it to the list, or shift if full
        if (index < MAX_POINTS) {
            points[index].x = r1;
            points[index].y = r2;
            index++;
        } else {
            // Shift all points left to make room for the new one
            for (int i = 1; i < MAX_POINTS; i++)
                points[i - 1] = points[i];
            points[MAX_POINTS - 1].x = r1;
            points[MAX_POINTS - 1].y = r2;
        }

        // Clear screen
        SDL_FillRect(surface, NULL, COLOR_BLACK);

        // Update and draw all points
        for (int i = 0; i < index; i++) {
            calculate_rect(&points[i], dt);  // Move point outward
            SDL_Rect p = {points[i].x, points[i].y, 4, 4};  // Star size 4x4
            SDL_FillRect(surface, &p, COLOR_WHITE);
        }

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}