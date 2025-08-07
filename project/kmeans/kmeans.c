#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define K 3 // clusters
#define MAX_ITERS 100
#define N 45 // number of points
#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 100
#define NUM_CELL (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)
#define ROWS HEIGHT / CELL_SIZE
#define COLUMNS WIDTH / CELL_SIZE

typedef struct
{
    double x, y;
    int cluster;
} Point;

typedef struct
{
    double x, y;
} Centroid;

double euclidean(Point point, Centroid centroid)
{
    return sqrt(pow(point.x - centroid.x, 2) + pow(point.y - centroid.y, 2));
}

void kmeans(Point data[], int n)
{
    Centroid centroids[K];

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
            double min_distance = euclidean(data[i], centroids[0]);
            int best_cluster = 0;
            for (int k = 0; k < K; k++)
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
        // new centroids
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
            printf("Convergence achieved after %d iterations.\n", iter);
            break;
        }
    }
    for (int i = 0; i < n; i++)
    {
        printf("Point (%.2f, %.2f) -> cluster %d\n", data[i].x, data[i].y, data[i].cluster);
    }
}

void draw_grid(SDL_Surface *surface, Uint32 color)
{
    SDL_Rect line;

    // Vertical lines
    for (int x = 100; x <= WIDTH - 100; x += CELL_SIZE)
    {
        line.x = x;
        line.y = 100;
        line.w = 1;
        line.h = HEIGHT - 200;
        SDL_FillRect(surface, &line, color);
    }

    // Horizontal lines
    for (int y = 100; y <= HEIGHT - 100; y += CELL_SIZE)
    {
        line.x = 100;
        line.y = y;
        line.w = WIDTH - 200;
        line.h = 1;
        SDL_FillRect(surface, &line, color);
    }
}

int main()
{
    Point data[N] = {
        // Cluster 1
        {1.0, 2.0},
        {1.1, 1.9},
        {0.9, 2.2},
        {1.2, 1.8},
        {0.8, 2.0},
        {1.3, 2.1},
        {1.0, 1.7},
        {1.2, 2.3},
        {0.7, 1.9},
        {1.1, 2.2},
        {0.9, 1.8},
        {1.3, 2.0},
        {0.8, 2.1},
        {1.1, 2.3},
        {1.0, 1.6},

        // Cluster 2
        {8.0, 2.0},
        {8.2, 2.1},
        {7.9, 2.3},
        {8.1, 2.5},
        {8.3, 2.7},
        {7.8, 2.4},
        {8.0, 2.9},
        {8.4, 2.8},
        {7.9, 3.0},
        {8.2, 2.6},
        {8.0, 3.2},
        {8.1, 2.3},
        {8.3, 2.0},
        {7.7, 2.6},
        {8.2, 3.1},

        // Cluster 3
        {9.0, 10.0},
        {9.2, 10.1},
        {8.9, 10.3},
        {9.1, 9.8},
        {9.3, 10.4},
        {8.8, 10.2},
        {9.0, 10.5},
        {9.4, 10.3},
        {8.7, 10.1},
        {9.2, 10.2},
        {9.0, 10.8},
        {9.1, 9.9},
        {9.3, 10.0},
        {8.9, 10.4},
        {9.2, 10.6},
    };

    for (int i = 0; i < N; i++)
    {
        data[i].cluster = -1; // initialization
    }

    kmeans(data, N);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "K-means",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    Uint32 color_gray = SDL_MapRGB(surface->format, 130, 130, 130);
    draw_grid(surface, color_gray);

    int running = 1;
    SDL_Event event;
    while (running)
    {
        // Event handling
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}