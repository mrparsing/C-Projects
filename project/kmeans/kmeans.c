#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define K 5 // clusters
#define MAX_ITERS 100
#define MAX_POINTS 1000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MARGIN 50

typedef struct
{
    double x, y;
    int cluster;
} Point;

typedef struct
{
    double x, y;
} Centroid;

SDL_Color cluster_colors[K] = {
    {255, 100, 100, 255}, // red
    {100, 255, 100, 255}, // green
    {100, 100, 255, 255}, // blue
    {255, 255, 100, 255}, // yellow
    {200, 50, 200, 255}};

double euclidean(Point point, Centroid centroid)
{
    return sqrt(pow(point.x - centroid.x, 2) + pow(point.y - centroid.y, 2));
}

/*
normalize the coordinates of 2D points so that
they are scaled and translated in a uniform range
*/
void normalize_points(Point data[], int n)
{
    double min_x = data[0].x, max_x = data[0].x;
    double min_y = data[0].y, max_y = data[0].y;

    for (int i = 1; i < n; i++)
    {
        if (data[i].x < min_x)
            min_x = data[i].x;
        if (data[i].x > max_x)
            max_x = data[i].x;
        if (data[i].y < min_y)
            min_y = data[i].y;
        if (data[i].y > max_y)
            max_y = data[i].y;
    }

    double range_x = max_x - min_x;
    double range_y = max_y - min_y;

    for (int i = 0; i < n; i++)
    {
        data[i].x = ((data[i].x - min_x) / range_x) * 10.0;
        data[i].y = ((data[i].y - min_y) / range_y) * 10.0;
    }
}

void kmeans(Point data[], int n, Centroid centroids[])
{
    srand(time(NULL));

    // random centroids
    for (int i = 0; i < K; i++)
    {
        int index = rand() % n;
        centroids[i].x = data[index].x;
        centroids[i].y = data[index].y;
    }

    for (int iter = 0; iter < MAX_ITERS; iter++)
    {
        int changed = 0;

        for (int i = 0; i < n; i++)
        {
            // calculate the distance of every point to every centroids
            // and take the minimum distance
            double min_distance = euclidean(data[i], centroids[0]);
            int best_cluster = 0;

            for (int k = 1; k < K; k++)
            {
                double dist = euclidean(data[i], centroids[k]);
                if (dist < min_distance)
                {
                    min_distance = dist;
                    best_cluster = k;
                }
            }

            if (data[i].cluster != best_cluster)
            {
                changed = 1;
                data[i].cluster = best_cluster;
            }
        }

        double sum_x[K] = {0}, sum_y[K] = {0};
        int count[K] = {0};

        for (int i = 0; i < n; i++)
        {
            int c = data[i].cluster;
            sum_x[c] += data[i].x;
            sum_y[c] += data[i].y;
            count[c]++;
        }

        for (int k = 0; k < K; k++)
        {
            if (count[k])
            {
                centroids[k].x = sum_x[k] / count[k];
                centroids[k].y = sum_y[k] / count[k];
            }
        }

        if (!changed)
        {
            printf("Convergence reached after %d iterations.\n", iter + 1);
            break;
        }
    }
}

void to_screen_coords(double x, double y, int *screen_x, int *screen_y)
{
    *screen_x = MARGIN + (int)((x / 10.0) * (WINDOW_WIDTH - 2 * MARGIN));
    *screen_y = WINDOW_HEIGHT - MARGIN - (int)((y / 10.0) * (WINDOW_HEIGHT - 2 * MARGIN));
}

void draw_grid(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = 0; i <= 10; i++)
    {
        int x = MARGIN + (i * (WINDOW_WIDTH - 2 * MARGIN)) / 10;
        SDL_RenderDrawLine(renderer, x, MARGIN, x, WINDOW_HEIGHT - MARGIN);
    }

    for (int i = 0; i <= 10; i++)
    {
        int y = MARGIN + (i * (WINDOW_HEIGHT - 2 * MARGIN)) / 10;
        SDL_RenderDrawLine(renderer, MARGIN, y, WINDOW_WIDTH - MARGIN, y);
    }

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    int axis_y = MARGIN + (5 * (WINDOW_HEIGHT - 2 * MARGIN)) / 10;
    for (int i = 0; i < 3; i++)
    {
        SDL_RenderDrawLine(renderer, MARGIN, axis_y + i - 1, WINDOW_WIDTH - MARGIN, axis_y + i - 1);
    }

    int axis_x = MARGIN + (5 * (WINDOW_WIDTH - 2 * MARGIN)) / 10;
    for (int i = 0; i < 3; i++)
    {
        SDL_RenderDrawLine(renderer, axis_x + i - 1, MARGIN, axis_x + i - 1, WINDOW_HEIGHT - MARGIN);
    }
}

void draw_point(SDL_Renderer *renderer, Point point)
{
    int screen_x, screen_y;
    to_screen_coords(point.x, point.y, &screen_x, &screen_y);

    SDL_Color color = cluster_colors[point.cluster % K];
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int y = -4; y <= 4; y++)
    {
        for (int x = -4; x <= 4; x++)
        {
            if (x * x + y * y <= 16)
            {
                SDL_RenderDrawPoint(renderer, screen_x + x, screen_y + y);
            }
        }
    }
}

void draw_centroid(SDL_Renderer *renderer, Centroid centroid, int cluster_id)
{
    int screen_x, screen_y;
    to_screen_coords(centroid.x, centroid.y, &screen_x, &screen_y);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    for (int y = -6; y <= 6; y++)
    {
        for (int x = -6; x <= 6; x++)
        {
            if (x * x + y * y <= 36)
            {
                SDL_RenderDrawPoint(renderer, screen_x + x, screen_y + y);
            }
        }
    }

    SDL_Color color = cluster_colors[cluster_id % K];
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -4; y <= 4; y++)
    {
        for (int x = -4; x <= 4; x++)
        {
            if (x * x + y * y <= 16)
            {
                SDL_RenderDrawPoint(renderer, screen_x + x, screen_y + y);
            }
        }
    }
}

int load_points_from_file(const char *filename, Point *points)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("Error opening file");
        return -1;
    }

    int count = 0;
    while (fscanf(fp, "%lf %lf", &points[count].x, &points[count].y) == 2)
    {
        points[count].cluster = -1;
        count++;
        if (count >= MAX_POINTS)
            break;
    }

    fclose(fp);
    return count;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <input_points.txt>\n", argv[0]);
        return 1;
    }

    Point data[MAX_POINTS];
    int N = load_points_from_file(argv[1], data);
    if (N <= 0)
    {
        printf("Error reading points or empty file\n");
        return 1;
    }

    Centroid centroids[K];

    normalize_points(data, N);
    kmeans(data, N, centroids);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Errore SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "K-means",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window)
    {
        printf("Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Error renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r)
            {
                for (int i = 0; i < N; i++)
                {
                    data[i].cluster = -1;
                }
                kmeans(data, N, centroids);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        draw_grid(renderer);

        for (int i = 0; i < N; i++)
        {
            if (data[i].cluster >= 0)
            {
                draw_point(renderer, data[i]);
            }
        }

        for (int k = 0; k < K; k++)
        {
            draw_centroid(renderer, centroids[k], k);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}