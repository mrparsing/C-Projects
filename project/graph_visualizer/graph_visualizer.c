#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NODE_RADIUS 20
#define MAX_NODES 100
#define MAX_EDGES 200
#define INF 0x3f3f3f3f

typedef struct
{
    int x, y;
    int id;
    int is_source;
    int is_goal;
} Node;

typedef struct
{
    int from;
    int to;
    int weight;
} Edge;

void draw_circle(SDL_Renderer *renderer, int x, int y, int radius)
{
    for (int w = 0; w < radius * 2; w++)
    {
        for (int h = 0; h < radius * 2; h++)
        {
            int dx = radius - w;
            int dy = radius - h;
            if (dx * dx + dy * dy <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void draw_arrow(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
{
    float angle = atan2(y2 - y1, x2 - x1);

    float offset = NODE_RADIUS;
    int new_x1 = x1 + offset * cos(angle);
    int new_y1 = y1 + offset * sin(angle);
    int new_x2 = x2 - offset * cos(angle);
    int new_y2 = y2 - offset * sin(angle);

    SDL_RenderDrawLine(renderer, new_x1, new_y1, new_x2, new_y2);

    float arrow_length = 12.0;
    float arrow_angle = M_PI / 6;

    float x3 = new_x2 - arrow_length * cos(angle - arrow_angle);
    float y3 = new_y2 - arrow_length * sin(angle - arrow_angle);
    float x4 = new_x2 - arrow_length * cos(angle + arrow_angle);
    float y4 = new_y2 - arrow_length * sin(angle + arrow_angle);

    SDL_RenderDrawLine(renderer, new_x2, new_y2, (int)x3, (int)y3);
    SDL_RenderDrawLine(renderer, new_x2, new_y2, (int)x4, (int)y4);
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, int x, int y, const char *text, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int is_point_in_node(int mx, int my, Node *node)
{
    int dx = mx - node->x;
    int dy = my - node->y;
    return (dx * dx + dy * dy) <= NODE_RADIUS * NODE_RADIUS;
}

void bellman_ford(Node nodes[], int nodes_count, Edge edges[], int edges_count,
                  int dist[], int pred[], int *has_negative_cycle)
{
    for (int i = 0; i < nodes_count; i++)
    {
        dist[i] = INF;
        pred[i] = -1;
    }

    int source_index = -1;
    for (int i = 0; i < nodes_count; i++)
    {
        if (nodes[i].is_source)
        {
            source_index = i;
            break;
        }
    }

    if (source_index == -1)
    {
        printf("No source node found!\n");
        return;
    }

    dist[source_index] = 0;

    for (int i = 1; i <= nodes_count - 1; i++)
    {
        for (int j = 0; j < edges_count; j++)
        {
            int u = edges[j].from;
            int v = edges[j].to;
            int weight = edges[j].weight;

            if (dist[u] != INF && dist[u] + weight < dist[v])
            {
                dist[v] = dist[u] + weight;
                pred[v] = u;
            }
        }
    }

    *has_negative_cycle = 0;
    for (int j = 0; j < edges_count; j++)
    {
        int u = edges[j].from;
        int v = edges[j].to;
        int weight = edges[j].weight;

        if (dist[u] != INF && dist[u] + weight < dist[v])
        {
            *has_negative_cycle = 1;
            printf("Negative cycle detected!\n");
            return;
        }
    }
}

void dijkstra(Node nodes[], Edge edges[], int dist[], int pred[], int nodes_count, int edges_count)
{
    int visited[nodes_count];
    for (int i = 0; i < nodes_count; i++)
    {
        dist[i] = INF;
        pred[i] = -1;
        visited[i] = 0;
    }

    int source_index = -1;
    for (int i = 0; i < nodes_count; i++)
    {
        if (nodes[i].is_source)
        {
            source_index = i;
            break;
        }
    }

    if (source_index == -1)
    {
        printf("No source node found!\n");
        return;
    }

    dist[source_index] = 0;

    for (int count = 0; count < nodes_count - 1; count++)
    {
        int u = -1;
        int min_dist = INF;
        for (int i = 0; i < nodes_count; i++)
        {
            if (!visited[i] && dist[i] < min_dist)
            {
                u = i;
                min_dist = dist[i];
            }
        }
        if (u == -1)
        {
            break;
        }
        visited[u] = 1;

        for (int i = 0; i < edges_count; i++)
        {
            if (edges[i].from == u)
            {
                int v = edges[i].to;
                int weight = edges[i].weight;
                if (!visited[v] && dist[u] + weight < dist[v])
                {
                    dist[v] = dist[u] + weight;
                    pred[v] = u;
                }
            }
        }
    }
}

int main()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Graph Visualizer",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    TTF_Font *font = TTF_OpenFont("FreeSansBold.otf", 16);
    if (!font)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Node nodes[MAX_NODES];
    int nodes_count = 0;

    Edge edges[MAX_EDGES];
    int edges_count = 0;

    int dist[MAX_NODES];
    int pred[MAX_NODES];
    int has_negative_cycle = 0;
    int path_nodes[MAX_NODES];
    int path_length = 0;
    const char *last_algorithm = "none";

    int running = 1;
    SDL_Event event;

    int shift_pressed = 0;
    int selected_node = -1;
    int dragging_node = -1;
    int offset_x = 0, offset_y = 0;
    int g_pressed = 0;
    int s_pressed = 0;

    SDL_Color white = {255, 255, 255, 255};

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
                    shift_pressed = 1;
                else if (event.key.keysym.sym == SDLK_g)
                    g_pressed = 1;
                else if (event.key.keysym.sym == SDLK_s)
                    s_pressed = 1;
                else if (event.key.keysym.sym == SDLK_b)
                {
                    bellman_ford(nodes, nodes_count, edges, edges_count,
                                 dist, pred, &has_negative_cycle);
                    last_algorithm = "bellman_ford";

                    path_length = 0;
                    int goal_index = -1;
                    for (int i = 0; i < nodes_count; i++)
                    {
                        if (nodes[i].is_goal)
                        {
                            goal_index = i;
                            break;
                        }
                    }

                    if (goal_index != -1 && !has_negative_cycle)
                    {
                        int current = goal_index;
                        while (current != -1 && path_length < MAX_NODES)
                        {
                            path_nodes[path_length++] = current;
                            current = pred[current];
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_d)
                {
                    dijkstra(nodes, edges, dist, pred, nodes_count, edges_count);
                    last_algorithm = "dijkstra";

                    path_length = 0;
                    int goal_index = -1;
                    for (int i = 0; i < nodes_count; i++)
                    {
                        if (nodes[i].is_goal)
                        {
                            goal_index = i;
                            break;
                        }
                    }

                    if (goal_index != -1)
                    {
                        int current = goal_index;
                        while (current != -1 && path_length < MAX_NODES)
                        {
                            path_nodes[path_length++] = current;
                            current = pred[current];
                        }
                    
                    }
                }
                break;

            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT)
                {
                    shift_pressed = 0;
                    selected_node = -1;
                }
                else if (event.key.keysym.sym == SDLK_g)
                    g_pressed = 0;
                else if (event.key.keysym.sym == SDLK_s)
                    s_pressed = 0;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int mx = event.button.x;
                    int my = event.button.y;

                    if (s_pressed || g_pressed)
                    {
                        for (int i = 0; i < nodes_count; i++)
                        {
                            if (is_point_in_node(mx, my, &nodes[i]))
                            {
                                if (s_pressed)
                                {
                                    for (int j = 0; j < nodes_count; j++)
                                        nodes[j].is_source = 0;
                                    nodes[i].is_source = 1;
                                }
                                else if (g_pressed)
                                {
                                    for (int j = 0; j < nodes_count; j++)
                                        nodes[j].is_goal = 0;
                                    nodes[i].is_goal = 1;
                                }
                                last_algorithm = "none";
                                break;
                            }
                        }
                    }
                    else if (shift_pressed)
                    {
                        for (int i = 0; i < nodes_count; i++)
                        {
                            if (is_point_in_node(mx, my, &nodes[i]))
                            {
                                if (selected_node == -1)
                                {
                                    selected_node = i;
                                }
                                else if (selected_node != i)
                                {
                                    if (edges_count < MAX_EDGES)
                                    {
                                        edges[edges_count].from = selected_node;
                                        edges[edges_count].to = i;
                                        edges[edges_count].weight = rand() % 20 + 1;
                                        edges_count++;
                                    }
                                    selected_node = -1;
                                    last_algorithm = "none";
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < nodes_count; i++)
                        {
                            if (is_point_in_node(mx, my, &nodes[i]))
                            {
                                dragging_node = i;
                                offset_x = nodes[i].x - mx;
                                offset_y = nodes[i].y - my;
                                break;
                            }
                        }
                    }
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    if (nodes_count < MAX_NODES)
                    {
                        nodes[nodes_count].x = event.button.x;
                        nodes[nodes_count].y = event.button.y;
                        nodes[nodes_count].id = nodes_count;
                        nodes[nodes_count].is_source = 0;
                        nodes[nodes_count].is_goal = 0;
                        nodes_count++;
                        last_algorithm = "none";
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    dragging_node = -1;
                break;

            case SDL_MOUSEMOTION:
                if (dragging_node != -1)
                {
                    int mx = event.motion.x;
                    int my = event.motion.y;
                    nodes[dragging_node].x = mx + offset_x;
                    nodes[dragging_node].y = my + offset_y;

                    if (nodes[dragging_node].x < NODE_RADIUS)
                        nodes[dragging_node].x = NODE_RADIUS;
                    if (nodes[dragging_node].y < NODE_RADIUS)
                        nodes[dragging_node].y = NODE_RADIUS;
                    if (nodes[dragging_node].x > WINDOW_WIDTH - NODE_RADIUS)
                        nodes[dragging_node].x = WINDOW_WIDTH - NODE_RADIUS;
                    if (nodes[dragging_node].y > WINDOW_HEIGHT - NODE_RADIUS)
                        nodes[dragging_node].y = WINDOW_HEIGHT - NODE_RADIUS;
                }
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < edges_count; i++)
        {
            int f = edges[i].from;
            int t = edges[i].to;

            int in_path = 0;
            if (strcmp(last_algorithm, "none") != 0 && path_length > 0)
            {
                for (int j = 0; j < path_length - 1; j++)
                {
                    if ((path_nodes[j] == f && path_nodes[j + 1] == t) ||
                        (path_nodes[j] == t && path_nodes[j + 1] == f))
                    {
                        in_path = 1;
                        break;
                    }
                }
            }

            if (in_path)
            {
                SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            }

            draw_arrow(renderer, nodes[f].x, nodes[f].y, nodes[t].x, nodes[t].y);

            int mid_x = (nodes[f].x + nodes[t].x) / 2;
            int mid_y = (nodes[f].y + nodes[t].y) / 2;
            char weight_text[10];
            snprintf(weight_text, sizeof(weight_text), "%d", edges[i].weight);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            draw_text(renderer, font, mid_x, mid_y, weight_text, white);
        }

        for (int i = 0; i < nodes_count; i++)
        {
            int in_path = 0;
            if (strcmp(last_algorithm, "none") != 0 && path_length > 0)
            {
                for (int j = 0; j < path_length; j++)
                {
                    if (path_nodes[j] == i)
                    {
                        in_path = 1;
                        break;
                    }
                }
            }

            if (nodes[i].is_source)
            {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            else if (nodes[i].is_goal)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
            else if (in_path)
            {
                SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            }

            draw_circle(renderer, nodes[i].x, nodes[i].y, NODE_RADIUS);

            char id_text[10];
            snprintf(id_text, sizeof(id_text), "%d", i);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            draw_text(renderer, font, nodes[i].x - 5, nodes[i].y - 6, id_text, white);
        }

        if (strcmp(last_algorithm, "bellman_ford") == 0)
        {
            if (has_negative_cycle)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                draw_text(renderer, font, 10, 10, "Negative cycle", white);
            }
            else
            {
                int goal_index = -1;
                for (int i = 0; i < nodes_count; i++)
                {
                    if (nodes[i].is_goal)
                    {
                        goal_index = i;
                        break;
                    }
                }
                if (goal_index != -1)
                {
                    char result_text[50];
                    if (dist[goal_index] == INF)
                    {
                        snprintf(result_text, sizeof(result_text), "Goal unreachable");
                    }
                    else
                    {
                        snprintf(result_text, sizeof(result_text), "Bellman-Ford -> Shortest path: %d", dist[goal_index]);
                    }
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                    draw_text(renderer, font, 10, 10, result_text, white);
                }
            }
        }
        else if (strcmp(last_algorithm, "dijkstra") == 0)
        {
            int goal_index = -1;
            for (int i = 0; i < nodes_count; i++)
            {
                if (nodes[i].is_goal)
                {
                    goal_index = i;
                    break;
                }
            }
            if (goal_index != -1)
            {
                char result_text[50];
                if (dist[goal_index] == INF)
                {
                    snprintf(result_text, sizeof(result_text), "Goal unreachable");
                }
                else
                {
                    snprintf(result_text, sizeof(result_text), "Dijkstra -> Shortest path: %d", dist[goal_index]);
                }
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                draw_text(renderer, font, 10, 10, result_text, white);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}