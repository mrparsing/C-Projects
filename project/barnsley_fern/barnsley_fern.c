#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600
#define ITERATION 200000

// Map a floating-point x coordinate from fern space to screen space
static inline int mapx(double x){
    const double xmin = -2.1820, xmax = 2.6558;
    return (int)((x - xmin) / (xmax - xmin) * (WIDTH - 1));
}

// Map a floating-point y coordinate from fern space to screen space (flipped)
static inline int mapy(double y){
    const double ymin = 0.0, ymax = 9.9983;
    return HEIGHT - 1 - (int)((y - ymin) / (ymax - ymin) * (HEIGHT - 1));
}

// Put a pixel on the SDL surface at (x, y) with a given color
static inline void putpixel(SDL_Surface *s, int x, int y, Uint32 color){
    Uint32 *pixels = (Uint32 *)s->pixels;
    pixels[y * (s->pitch / 4) + x] = color;
}

// Generate and draw the Barnsley Fern
void draw_barnsley(SDL_Surface *surface){
    double x = 0.0, y = 0.0;
    Uint32 white = SDL_MapRGBA(surface->format, 255, 255, 255, 255);

    // Clear the surface (black background)
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 255));
    SDL_LockSurface(surface);  // Lock for direct pixel access

    for (int i = 0; i < ITERATION; ++i){
        double nx, ny;
        int r = rand() % 100;

        // Apply one of the 4 affine transformations based on probability
        if (r < 1){
            nx = 0;
            ny = 0.16 * y;
        } else if (r < 86){
            nx = 0.85 * x + 0.04 * y;
            ny = -0.04 * x + 0.85 * y + 1.6;
        } else if (r < 93){
            nx = 0.2 * x - 0.26 * y;
            ny = 0.23 * x + 0.22 * y + 1.6;
        } else {
            nx = -0.15 * x + 0.28 * y;
            ny = 0.26 * x + 0.24 * y + 0.44;
        }

        // Update point
        x = nx;
        y = ny;

        // Convert to pixel coordinates and draw
        int px = mapx(x);
        int py = mapy(y);
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT)
            putpixel(surface, px, py, white);
    }

    SDL_UnlockSurface(surface);
}

int main(void){
    srand((unsigned)time(NULL)); // Seed the random number generator

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Barnsley Fern",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!win){
        fprintf(stderr, "CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surf = SDL_GetWindowSurface(win);
    if (!surf){
        fprintf(stderr, "GetWindowSurface error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    draw_barnsley(surf);
    SDL_UpdateWindowSurface(win);

    // Event loop to keep the window open
    SDL_Event e;
    int running = 1;
    while (running){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT)
                running = 0;
        }
        SDL_Delay(16); // ~60 FPS idle loop
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}