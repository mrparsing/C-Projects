#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH  900
#define HEIGHT 600
#define COLOR_BLACK 0xFF000000
#define COLOR_WHITE 0xffffffff
#define COLOR_GRAY 0xefefefef
#define RAYS_NUMBER 200

// Circle structure
struct Circle
{
	double x;
	double y;
	double r;
};

// Ray structure
struct Ray
{
	double x_start, y_start;
	double x_end, y_end;
	double angle;
};

// Method to draw a filled circle
// The circle is drawn by iterating through a square around the center
// If the distance of a pixel from the center is less than the radius, the pixel is colored
void FillCircle(SDL_Surface* surface, struct Circle circle) {
	double radius_squared = pow(circle.r, 2);
	
	for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++) {
		for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++) {
			double distance_squared_center = pow(x - circle.x, 2) + pow(y - circle.y, 2);
			if (distance_squared_center < radius_squared) {
				SDL_Rect pixel = (SDL_Rect) {x, y, 1, 1};
				SDL_FillRect(surface, &pixel, COLOR_WHITE);
			}
		}
	}
}

// Bresenham's line drawing algorithm to draw a line pixel by pixel
void DrawLine(SDL_Surface* surface, int x0, int y0, int x1, int y1, Uint32 color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        // Draw the pixel
        if (x0 >= 0 && x0 < surface->w && y0 >= 0 && y0 < surface->h) {
            Uint32* pixels = (Uint32*)surface->pixels;
            pixels[(y0 * surface->w) + x0] = color;
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

// Draw all rays to the surface
void FillRays(SDL_Surface* surface, struct Ray rays[RAYS_NUMBER]) {
    for (int i = 0; i < RAYS_NUMBER; i++) {
        int x0 = (int)rays[i].x_start;
        int y0 = (int)rays[i].y_start;
        int x1 = (int)rays[i].x_end;
        int y1 = (int)rays[i].y_end;
        DrawLine(surface, x0, y0, x1, y1, COLOR_GRAY);
    }
}

// Generate RAYS_NUMBER rays radiating from the center of the given circle
void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER])
{
	for (int i = 0; i < RAYS_NUMBER; i++) {
	    double ang = ((double)i / RAYS_NUMBER) * 2 * M_PI;
	    rays[i].x_start = circle.x;
	    rays[i].y_start = circle.y;
	    rays[i].x_end = (int)(rays[i].x_start + cos(ang) * 1000);
	    rays[i].y_end = (int)(rays[i].y_start + sin(ang) * 1000);
	    rays[i].angle = ang;
	}
}

// Check if each ray collides with the given circle
// If it does, adjust the end point to the intersection point
void check_collision(struct Circle circle, struct Ray rays[], int ray_count)
{
    for (int i = 0; i < ray_count; i++)
    {
        double dx = rays[i].x_end - rays[i].x_start;
        double dy = rays[i].y_end - rays[i].y_start;
        double fx = rays[i].x_start - circle.x;
        double fy = rays[i].y_start - circle.y;

        double a = dx*dx + dy*dy;
        double b = 2 * (fx*dx + fy*dy);
        double c = fx*fx + fy*fy - circle.r * circle.r;

        double discriminant = b*b - 4*a*c;

        if (discriminant < 0) {
            continue; // No intersection
        }

        discriminant = sqrt(discriminant);

        double t1 = (-b - discriminant) / (2*a);
        double t2 = (-b + discriminant) / (2*a);

        // If the intersection is within the segment [0,1], update the ray's end point
        if (t1 >= 0 && t1 <= 1) {
            rays[i].x_end = rays[i].x_start + t1 * dx;
            rays[i].y_end = rays[i].y_start + t1 * dy;
        } else if (t2 >= 0 && t2 <= 1) {
            rays[i].x_end = rays[i].x_start + t2 * dx;
            rays[i].y_end = rays[i].y_start + t2 * dy;
        }
    }
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Raytracing",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN
    );

    SDL_Surface *surface = SDL_GetWindowSurface(window);

	SDL_Rect erase_rect = {0, 0, WIDTH, HEIGHT};
	struct Circle circle = {200, 200, 50};              // Movable circle (follows the mouse)
	struct Circle shadow_circle = {600, 300, 110};      // Static obstacle
	struct Ray rays[RAYS_NUMBER];
	
	int running = 1;
	
	SDL_Event e;
	while(running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = 0;
			}
			// Move the circle with the mouse if a button is pressed
			if (e.type == SDL_MOUSEMOTION && e.motion.state != 0)
			{
				circle.x = e.motion.x;
				circle.y = e.motion.y;
			}
		}
		// Clear the screen
		SDL_FillRect(surface, &erase_rect, COLOR_BLACK);

		// Draw both circles
		FillCircle(surface, circle);
		FillCircle(surface, shadow_circle);
		
		// Generate rays from the moving circle
		generate_rays(circle, rays);

		// Check which rays collide with the static circle and update them
		check_collision(shadow_circle, rays, RAYS_NUMBER);

		// Draw the rays
		FillRays(surface, rays);
		
		// Update the window
		SDL_UpdateWindowSurface(window);
		SDL_Delay(10);
	}

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}