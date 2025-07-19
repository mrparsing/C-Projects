#include <stdio.h>
#include <SDL2/SDL.h>
#include <complex.h>

#define WIDTH 900
#define HEIGHT 600
#define MAX_ITER 500

// Function to convert HSV (0–360°, 0–1, 0–1) to RGB 0–255
void hsv_to_rgb(float h, float s, float v, Uint8 *r, Uint8 *g, Uint8 *b) {
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - c;
    float rp, gp, bp;
    if (h < 60)       { rp = c; gp = x; bp = 0; }
    else if (h < 120) { rp = x; gp = c; bp = 0; }
    else if (h < 180) { rp = 0; gp = c; bp = x; }
    else if (h < 240) { rp = 0; gp = x; bp = c; }
    else if (h < 300) { rp = x; gp = 0; bp = c; }
    else              { rp = c; gp = 0; bp = x; }
    *r = (Uint8)((rp + m) * 255);
    *g = (Uint8)((gp + m) * 255);
    *b = (Uint8)((bp + m) * 255);
}

void visualize_mandelbrot(SDL_Surface *surface) {
    const double re_min = -2.0, re_max = 1.0;
    const double im_min = -1.0, im_max = 1.0;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            double real = re_min + x * (re_max - re_min) / WIDTH;
            double imag = im_max - y * (im_max - im_min) / HEIGHT;
            double _Complex z = 0 + 0*I;
            double _Complex c = real + imag * I;

            int iter = 0;
            while (iter < MAX_ITER && cabs(z) <= 2.0) {
                z = cpow(z, 2) + c;
                iter++;
            }

            Uint8 r, g, b;
            if (iter == MAX_ITER) {
                // Inside the set: black
                r = g = b = 0;
            } else {
                // Outside: map iter → hue (0°–360°)
                float hue = (360.0f * iter) / MAX_ITER;
                hsv_to_rgb(hue, 1.0f, 1.0f, &r, &g, &b);
            }

            Uint32 color = SDL_MapRGB(surface->format, r, g, b);
            SDL_Rect pixel = { x, y, 1, 1 };
            SDL_FillRect(surface, &pixel, color);
        }
    }
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!win) {
        fprintf(stderr, "CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surf = SDL_GetWindowSurface(win);
    if (!surf) {
        fprintf(stderr, "GetWindowSurface error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        visualize_mandelbrot(surf);
        SDL_UpdateWindowSurface(win);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}