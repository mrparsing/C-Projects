#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_YELLOW 0xffff00
#define COLOR_BLACK 0x00000000
#define COLOR_LIGHTBLUE 0x007fff
#define G 10000
#define EPSILON 1e-1
#define MAX_TRAIL_POINTS 500

struct Point {
    int x1, y1;
    int x2, y2;
};

struct Planet {
    double x;
    double y;
    double r;
    double vx;
    double vy;
    double mass;
};

void FillCircle(SDL_Surface *surface, struct Planet Planet, Uint32 color) {
    double radius_squared = pow(Planet.r, 2);
    for (double x = Planet.x - Planet.r; x <= Planet.x + Planet.r; x++) {
        for (double y = Planet.y - Planet.r; y <= Planet.y + Planet.r; y++) {
            double distance_squared_center = pow(x - Planet.x, 2) + pow(y - Planet.y, 2);
            if (distance_squared_center < radius_squared) {
                SDL_Rect pixel = (SDL_Rect){x, y, 1, 1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void calculate_distance(struct Planet *p1, struct Planet *p2, double dt, SDL_Surface *surface) {
    double dx = p2->x - p1->x;
    double dy = p2->y - p1->y;
    double r = sqrt(dx * dx + dy * dy + EPSILON);

    double F = (G * p1->mass * p2->mass) / (r * r);

    double ax1 = F * dx / r / p1->mass;
    double ay1 = F * dy / r / p1->mass;
    double ax2 = -F * dx / r / p2->mass;
    double ay2 = -F * dy / r / p2->mass;

    p1->vx += ax1 * dt;
    p1->vy += ay1 * dt;
    p2->vx += ax2 * dt;
    p2->vy += ay2 * dt;

    p1->x += p1->vx * dt;
    p1->y += p1->vy * dt;
    p2->x += p2->vx * dt;
    p2->y += p2->vy * dt;
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Orbiting planets simulation",
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

    struct Point trail[MAX_TRAIL_POINTS];
    int trail_index = 0;

    struct Planet planet2 = {
        WIDTH / 6.0, HEIGHT / 2.0,
        30.0,
        20.0, 0.0,
        300.0
    };

    double radius = 150.0;
    struct Planet planet1 = {
        planet2.x + radius,
        planet2.y,
        10.0,
        0.0, 0.0,
        5.0
    };

    double dx = planet1.x - planet2.x;
    double dy = planet1.y - planet2.y;
    double distance = sqrt(dx * dx + dy * dy);
    double speed = sqrt(G * planet2.mass / distance);

    planet1.vx = -dy / distance * speed;
    planet1.vy =  dx / distance * speed;

    Uint32 prev_ticks = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        double dt = (now - prev_ticks) / 1000.0;
        prev_ticks = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }

        calculate_distance(&planet1, &planet2, dt, surface);

        if (trail_index < MAX_TRAIL_POINTS) {
            trail[trail_index].x1 = (int)planet1.x;
            trail[trail_index].y1 = (int)planet1.y;
            trail[trail_index].x2 = (int)planet2.x;
            trail[trail_index].y2 = (int)planet2.y;
            trail_index++;
        } else {
            for (int i = 1; i < MAX_TRAIL_POINTS; i++)
                trail[i - 1] = trail[i];
            trail[MAX_TRAIL_POINTS - 1].x1 = (int)planet1.x;
            trail[MAX_TRAIL_POINTS - 1].y1 = (int)planet1.y;
            trail[MAX_TRAIL_POINTS - 1].x2 = (int)planet2.x;
            trail[MAX_TRAIL_POINTS - 1].y2 = (int)planet2.y;
        }

        SDL_FillRect(surface, NULL, COLOR_BLACK);

        FillCircle(surface, planet1, COLOR_LIGHTBLUE);
        FillCircle(surface, planet2, COLOR_YELLOW);

        for (int i = 0; i < trail_index; i++) {
            SDL_Rect pixel1 = {trail[i].x1, trail[i].y1, 2, 2};
            SDL_Rect pixel2 = {trail[i].x2, trail[i].y2, 2, 2};
            SDL_FillRect(surface, &pixel1, COLOR_LIGHTBLUE);
            SDL_FillRect(surface, &pixel2, COLOR_YELLOW);
        }

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}