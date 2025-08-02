#include <SDL2/SDL.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NODE_RADIUS 20
#define MAX_NODES 100
#define MAX_EDGES 200

typedef struct {
    int x, y;
    int id;
} Node;

typedef struct {
    int from;
    int to;
} Edge;

void draw_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void draw_line(SDL_Renderer *renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

int is_point_in_node(int mx, int my, Node *node) {
    int dx = mx - node->x;
    int dy = my - node->y;
    return (dx * dx + dy * dy) <= NODE_RADIUS * NODE_RADIUS;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Graph Visualizer",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Node nodes[MAX_NODES];
    int nodes_count = 0;

    Edge edges[MAX_EDGES];
    int edges_count = 0;

    int running = 1;
    SDL_Event event;

    int dragging_node = -1;
    int offset_x = 0, offset_y = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x;
                        int my = event.button.y;
                        for (int i = 0; i < nodes_count; i++) {
                            if (is_point_in_node(mx, my, &nodes[i])) {
                                dragging_node = i;
                                offset_x = nodes[i].x - mx;
                                offset_y = nodes[i].y - my;
                                break;
                            }
                        }
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        if (nodes_count < MAX_NODES) {
                            nodes[nodes_count].x = event.button.x;
                            nodes[nodes_count].y = event.button.y;
                            nodes[nodes_count].id = nodes_count;
                            nodes_count++;
                            printf("Nodo aggiunto in (%d,%d), id=%d\n", event.button.x, event.button.y, nodes_count-1);
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging_node = -1;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (dragging_node != -1) {
                        int mx = event.motion.x;
                        int my = event.motion.y;
                        nodes[dragging_node].x = mx + offset_x;
                        nodes[dragging_node].y = my + offset_y;

                        if (nodes[dragging_node].x < NODE_RADIUS) nodes[dragging_node].x = NODE_RADIUS;
                        if (nodes[dragging_node].y < NODE_RADIUS) nodes[dragging_node].y = NODE_RADIUS;
                        if (nodes[dragging_node].x > WINDOW_WIDTH - NODE_RADIUS) nodes[dragging_node].x = WINDOW_WIDTH - NODE_RADIUS;
                        if (nodes[dragging_node].y > WINDOW_HEIGHT - NODE_RADIUS) nodes[dragging_node].y = WINDOW_HEIGHT - NODE_RADIUS;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < edges_count; i++) {
            int f = edges[i].from;
            int t = edges[i].to;
            draw_line(renderer, nodes[f].x, nodes[f].y, nodes[t].x, nodes[t].y);
        }

        for (int i = 0; i < nodes_count; i++) {
            SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
            draw_circle(renderer, nodes[i].x, nodes[i].y, NODE_RADIUS);

            SDL_SetRenderDrawColor(renderer, 0, 100, 180, 255);
            for (int r = NODE_RADIUS - 3; r <= NODE_RADIUS; r++) {
                draw_circle(renderer, nodes[i].x, nodes[i].y, r);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}