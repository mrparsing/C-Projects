/**
 * @file main.c
 * @brief This program simulates the gravitational interactions of three celestial bodies:
 * two stars and a central supermassive black hole. The rendering is achieved through
 * ray tracing, which calculates the path of light rays as they are bent by the
 * black hole's gravity, according to the principles of general relativity (using
 * the Schwarzschild metric).

 * Controls:
 * - Left Mouse + Drag: Orbit the camera.
 * - Middle Mouse + Drag: Pan the camera.
 * - Mouse Wheel: Zoom in/out.
 * - 'R': Reset the camera to its initial state.
 * - 'P': Pause or resume the physics simulation.
 * - 'G': Toggle the visibility of the spacetime grid.
 * - 'ESC': Exit the application.
 */


// INCLUDES & DEFINITIONS

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// SIMULATION AND PHYSICAL CONSTANTS
static const double SPEED_OF_LIGHT = 299792458.0;
static const double GRAVITATIONAL_CONSTANT = 6.67430e-11;
static const float BLACK_HOLE_SCHWARZSCHILD_RADIUS = 1.269e10f;
static float RAY_INTEGRATION_STEP = 5e7f; // Initial step size for ray integration
static const double RAY_ESCAPE_RADIUS = 1e30;    // Radius at which rays are considered to have escaped
static const int NUM_CELESTIAL_BODIES = 3;
static bool is_physics_paused = false;
static bool is_grid_visible = true;

// STRUCTURE DEFINITIONS
/**
 * @struct vector3_t
 * @brief Represents a 3D vector.
 */
typedef struct
{
    float x, y, z;
} vector3_t;

/**
 * @struct vector4_t
 * @brief Represents a 4D vector.
 */
typedef struct
{
    float x, y, z, w;
} vector4_t;

/**
 * @struct matrix4_t
 * @brief Represents a 4x4 matrix in column-major order.
 */
typedef struct
{
    float elements[16];
} matrix4_t;

/**
 * @struct camera_t
 * @brief Represents the state of the interactive orbital camera.
 */
typedef struct
{
    vector3_t target;
    float radius;
    float min_radius, max_radius;
    float azimuth;
    float elevation;
    float orbit_speed;
    float pan_speed;
    double zoom_speed;
    bool is_dragging_orbit;
    bool is_dragging_pan;
    bool is_moving;
    double last_cursor_x, last_cursor_y;
} camera_t;

/**
 * @struct celestial_body_t
 * @brief Represents a celestial body in the simulation.
 */
typedef struct
{
    vector4_t position_and_radius; // .xyz for position, .w for radius
    vector4_t color;
    float mass;
    vector3_t velocity;
} celestial_body_t;

/**
 * @struct renderer_engine_t
 * @brief Encapsulates all OpenGL and GLFW objects required for rendering.
 */
typedef struct
{
    GLFWwindow *window;
    GLuint fullscreen_quad_vao;
    GLuint render_texture;
    GLuint raytracer_shader_program;
    GLuint grid_shader_program;
    GLuint texture_quad_shader_program;
    GLuint grid_vao, grid_vbo, grid_ebo;
    int grid_index_count;
    int window_width, window_height;
    int render_texture_width, render_texture_height;
} renderer_engine_t;

// GLOBAL STATE
const camera_t initial_camera_state = {
    .target = {0.0f, 0.0f, 0.0f},
    .radius = 17.0e10f,
    .min_radius = 1e10f,
    .max_radius = 25.0e10f,
    .azimuth = 0.0f,
    .elevation = M_PI / 2.4f,
    .orbit_speed = 0.01f,
    .pan_speed = 0.005f,
    .zoom_speed = 25e9f,
    .is_dragging_orbit = false,
    .is_dragging_pan = false,
    .is_moving = false,
    .last_cursor_x = 0.0,
    .last_cursor_y = 0.0};
static camera_t camera;

static celestial_body_t celestial_bodies[] = {
    {{2.3e11f, 0.0f, 0.0f, 4e10f},   // Position and radius
     {0.4, 0.7, 1.0, 1.0},           // Color (Blue star)
     1.98892e30f,                    // Mass (Solar mass)
     {0.0f, 0.0f, 5.34e7}},          // Initial velocity
    {{-1.6e11f, 0.0f, 0.0f, 4e10f},  // Position and radius
     {0.8, 0.3, 0.2, 1.0},           // Color (Red star)
     1.98892e30f,                    // Mass (Solar mass)
     {0.0f, 0.0f, -5.34e7}},         // Initial velocity
    {{0.0f, 0.0f, 0.0f, BLACK_HOLE_SCHWARZSCHILD_RADIUS}, // Position and radius
     {0, 0, 0, 1},                   // Color (Black hole)
     8.54e36f,                       // Mass (Supermassive)
     {0, 0, 0}}                      // Initial velocity
};

static renderer_engine_t renderer_engine;

// MATH UTILITY FUNCTIONS

/**
 * @brief Clamps a float value between a minimum and a maximum.
 */
float utility_clamp_float(float value, float min_val, float max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/**
 * @brief Calculates the magnitude of a 3D vector.
 */
float vector3_length(vector3_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Normalizes a 3D vector to unit length.
 */
vector3_t vector3_normalize(vector3_t v)
{
    float len = vector3_length(v);
    if (len > 0.0f)
    {
        return (vector3_t){v.x / len, v.y / len, v.z / len};
    }
    return (vector3_t){0.0f, 0.0f, 0.0f};
}

/**
 * @brief Computes the cross product of two 3D vectors.
 */
vector3_t vector3_cross(vector3_t a, vector3_t b)
{
    return (vector3_t){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

/**
 * @brief Subtracts vector b from vector a.
 */
vector3_t vector3_subtract(vector3_t a, vector3_t b)
{
    return (vector3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

/**
 * @brief Adds two 3D vectors.
 */
vector3_t vector3_add(vector3_t a, vector3_t b)
{
    return (vector3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

/**
 * @brief Scales a 3D vector by a scalar value.
 */
vector3_t vector3_scale(vector3_t v, float s)
{
    return (vector3_t){v.x * s, v.y * s, v.z * s};
}

/**
 * @brief Creates a 4x4 identity matrix.
 */
matrix4_t matrix4_identity()
{
    matrix4_t m = {0};
    m.elements[0] = m.elements[5] = m.elements[10] = m.elements[15] = 1.0f;
    return m;
}

/**
 * @brief Creates a perspective projection matrix.
 */
matrix4_t matrix4_perspective(float fovy, float aspect, float z_near, float z_far)
{
    matrix4_t result = {0};
    float tan_half_fovy = tanf(fovy / 2.0f);
    result.elements[0] = 1.0f / (aspect * tan_half_fovy);
    result.elements[5] = 1.0f / tan_half_fovy;
    result.elements[10] = -(z_far + z_near) / (z_far - z_near);
    result.elements[11] = -1.0f;
    result.elements[14] = -(2.0f * z_far * z_near) / (z_far - z_near);
    return result;
}

/**
 * @brief Creates a view matrix to look at a target from a given position.
 */
matrix4_t matrix4_look_at(vector3_t eye, vector3_t center, vector3_t up)
{
    vector3_t f = vector3_normalize(vector3_subtract(center, eye));
    vector3_t s = vector3_normalize(vector3_cross(f, up));
    vector3_t u = vector3_cross(s, f);
    matrix4_t result = matrix4_identity();
    result.elements[0] = s.x;
    result.elements[4] = s.y;
    result.elements[8] = s.z;
    result.elements[1] = u.x;
    result.elements[5] = u.y;
    result.elements[9] = u.z;
    result.elements[2] = -f.x;
    result.elements[6] = -f.y;
    result.elements[10] = -f.z;
    result.elements[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    result.elements[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    result.elements[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
    return result;
}

/**
 * @brief Multiplies two 4x4 matrices.
 */
matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b)
{
    matrix4_t result = {0};
    for (int c = 0; c < 4; ++c)
    {
        for (int r = 0; r < 4; ++r)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                sum += a.elements[k * 4 + r] * b.elements[c * 4 + k];
            }
            result.elements[c * 4 + r] = sum;
        }
    }
    return result;
}

// CAMERA FUNCTIONS

/**
 * @brief Calculates the camera's world position based on its orbital parameters.
 */
vector3_t camera_get_position(const camera_t *cam)
{
    // Clamp elevation to avoid gimbal lock at the poles
    float clamped_elevation = utility_clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    vector3_t orbital_pos = {
        cam->radius * sinf(clamped_elevation) * cosf(cam->azimuth),
        cam->radius * cosf(clamped_elevation),
        cam->radius * sinf(clamped_elevation) * sinf(cam->azimuth)};
    return vector3_add(cam->target, orbital_pos);
}

/**
 * @brief Updates the camera's moving state.
 */
void camera_update_moving_state(camera_t *cam)
{
    cam->is_moving = cam->is_dragging_orbit || cam->is_dragging_pan;
}

/**
 * @brief Processes mouse movement to update camera orientation and position.
 */
void camera_process_mouse_move(camera_t *cam, double x, double y)
{
    float dx = (float)(x - cam->last_cursor_x);
    float dy = (float)(y - cam->last_cursor_y);

    if (cam->is_dragging_orbit)
    {
        cam->azimuth += dx * cam->orbit_speed;
        cam->elevation -= dy * cam->orbit_speed;
        cam->elevation = utility_clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    }

    if (cam->is_dragging_pan)
    {
        vector3_t pos = camera_get_position(cam);
        vector3_t fwd = vector3_normalize(vector3_subtract(cam->target, pos));
        vector3_t global_up = {0, 1, 0};
        vector3_t right = vector3_normalize(vector3_cross(fwd, global_up));
        vector3_t up = vector3_normalize(vector3_cross(right, fwd));

        // scale pan speed with distance to maintain consistent feel
        float pan_scale = cam->radius / 1e11f;
        vector3_t pan_offset_x = vector3_scale(right, -dx * cam->pan_speed * pan_scale);
        vector3_t pan_offset_y = vector3_scale(up, dy * cam->pan_speed * pan_scale);
        cam->target = vector3_add(cam->target, pan_offset_x);
        cam->target = vector3_add(cam->target, pan_offset_y);
    }

    cam->last_cursor_x = x;
    cam->last_cursor_y = y;
    camera_update_moving_state(cam);
}

/**
 * @brief Processes mouse scroll events to zoom the camera.
 */
void camera_process_scroll(camera_t *cam, double y_offset)
{
    cam->radius -= (float)(y_offset * cam->zoom_speed);
    cam->radius = utility_clamp_float(cam->radius, cam->min_radius, cam->max_radius);
    camera_update_moving_state(cam);
}

/**
 * @brief Resets the camera to its default state.
 */
void camera_reset(camera_t *cam)
{
    *cam = initial_camera_state;
}

// PHYSICS SIMULATION

/**
 * @brief Updates the positions and velocities of all celestial bodies based on
 * gravitational forces.
 * @param delta_time The time elapsed since the last update.
 */
void simulation_update_physics(double delta_time)
{
    if (is_physics_paused)
        return;

    // N-body simulation using Newton's law of universal gravitation.
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        for (int j = 0; j < NUM_CELESTIAL_BODIES; ++j)
        {
            if (i == j)
                continue;

            // Calculate distance and direction between bodies
            float dx = celestial_bodies[j].position_and_radius.x - celestial_bodies[i].position_and_radius.x;
            float dy = celestial_bodies[j].position_and_radius.y - celestial_bodies[i].position_and_radius.y;
            float dz = celestial_bodies[j].position_and_radius.z - celestial_bodies[i].position_and_radius.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            // Avoid division by zero and collision
            if (distance > (celestial_bodies[i].position_and_radius.w + celestial_bodies[j].position_and_radius.w))
            {
                vector3_t direction = {dx / distance, dy / distance, dz / distance};
                double gravitational_force = (GRAVITATIONAL_CONSTANT * celestial_bodies[i].mass * celestial_bodies[j].mass) / (distance * distance);
                double acceleration = gravitational_force / celestial_bodies[i].mass;

                // Update velocity based on acceleration
                celestial_bodies[i].velocity.x += direction.x * acceleration * delta_time;
                celestial_bodies[i].velocity.y += direction.y * acceleration * delta_time;
                celestial_bodies[i].velocity.z += direction.z * acceleration * delta_time;
            }
        }
    }
    // Update positions based on new velocities
    for (int i = 0; i < NUM_CELESTIAL_BODIES; i++)
    {
        celestial_bodies[i].position_and_radius.x += celestial_bodies[i].velocity.x * delta_time;
        celestial_bodies[i].position_and_radius.y += celestial_bodies[i].velocity.y * delta_time;
        celestial_bodies[i].position_and_radius.z += celestial_bodies[i].velocity.z * delta_time;
    }
}

// SPACETIME GRID RENDERING

/**
 * @brief Generates or updates the vertex data for the spacetime grid visualization.
 * The grid is deformed based on the gravitational potential of the celestial bodies.
 */
void grid_generate_mesh(renderer_engine_t *engine)
{
    const int grid_size = 50;
    const float grid_spacing = 1e10f;
    const double planet_curvature_scale = 500.0; // scaling for visual effect

    vector3_t *vertices = malloc((grid_size + 1) * (grid_size + 1) * sizeof(vector3_t));
    GLuint *indices = malloc(grid_size * grid_size * 4 * sizeof(GLuint));

    int vertex_count = 0;
    int index_count = 0;

    for (int z = 0; z <= grid_size; ++z)
    {
        for (int x = 0; x <= grid_size; ++x)
        {
            float world_x = (x - grid_size / 2) * grid_spacing;
            float world_z = (z - grid_size / 2) * grid_spacing;
            float y = -25e10f;

            for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
            {
                vector3_t obj_pos = {celestial_bodies[i].position_and_radius.x, celestial_bodies[i].position_and_radius.y, celestial_bodies[i].position_and_radius.z};
                double mass = celestial_bodies[i].mass;
                double schwarzschild_radius = 2.0 * GRAVITATIONAL_CONSTANT * mass / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
                double dx = world_x - obj_pos.x;
                double dz = world_z - obj_pos.z;
                double dist_sq = dx * dx + dz * dz;

                double delta_y = 0.0;
                if (dist_sq > schwarzschild_radius * schwarzschild_radius)
                {
                    double dist = sqrt(dist_sq);
                    // visual approximation of spacetime curvature (Flamm's paraboloid)
                    delta_y = sqrt(8.0 * schwarzschild_radius * (dist - schwarzschild_radius));
                    if (i != 2) // Scale down effect for non-black-hole objects
                    {
                        delta_y *= planet_curvature_scale;
                    }
                }
                y += (float)delta_y;
            }
            vertices[vertex_count++] = (vector3_t){world_x, y, world_z};
        }
    }

    for (int z = 0; z < grid_size; ++z)
    {
        for (int x = 0; x < grid_size; ++x)
        {
            int i = z * (grid_size + 1) + x;
            indices[index_count++] = i;
            indices[index_count++] = i + 1;
            indices[index_count++] = i;
            indices[index_count++] = i + grid_size + 1;
        }
    }
    
    if (engine->grid_vao == 0)
    {
        glGenVertexArrays(1, &engine->grid_vao);
        glGenBuffers(1, &engine->grid_vbo);
        glGenBuffers(1, &engine->grid_ebo);
    }

    glBindVertexArray(engine->grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, engine->grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vector3_t), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine->grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vector3_t), (void *)0);
    engine->grid_index_count = index_count;
    glBindVertexArray(0);
    
    free(vertices);
    free(indices);
}

/**
 * @brief Renders the spacetime grid.
 */
void grid_render(renderer_engine_t *engine, matrix4_t view_projection_matrix)
{
    if (!is_grid_visible) return;

    glUseProgram(engine->grid_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(engine->grid_shader_program, "viewProj"),
                       1, GL_FALSE, view_projection_matrix.elements);
    glBindVertexArray(engine->grid_vao);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_LINES, engine->grid_index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}


// OPENGL SHADER UTILITIES

/**
 * @brief Compiles a shader from a source string.
 * @return Shader ID, or 0 on failure.
 */
GLuint utility_compile_shader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("Shader compilation failed: %s\n", info_log);
        return 0;
    }
    return shader;
}

/**
 * @brief Creates a shader program by linking vertex and fragment shaders.
 * @return Program ID, or 0 on failure.
 */
GLuint utility_create_shader_program(const char *vertex_source, const char *fragment_source)
{
    GLuint vertex_shader = utility_compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = utility_compile_shader(fragment_source, GL_FRAGMENT_SHADER);

    if (!vertex_shader || !fragment_shader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Shader program linking failed: %s\n", info_log);
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

// RENDERER ENGINE FUNCTIONS

extern const char *quad_vertex_shader_source;
extern const char *quad_fragment_shader_source;
extern const char *grid_vertex_shader_source;
extern const char *grid_fragment_shader_source;
extern const char *raytracer_fragment_shader_source;

/**
 * @brief Initializes a Vertex Array Object for drawing a fullscreen quad.
 */
void engine_init_fullscreen_quad(renderer_engine_t *engine)
{
    float quad_vertices[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    GLuint vbo;
    glGenVertexArrays(1, &engine->fullscreen_quad_vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(engine->fullscreen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

/**
 * @brief Initializes the texture used as a render target for the ray tracer.
 */
void engine_init_render_texture(renderer_engine_t *engine)
{
    glGenTextures(1, &engine->render_texture);
    glBindTexture(GL_TEXTURE_2D, engine->render_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, engine->render_texture_width, engine->render_texture_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief Renders the main scene using the ray tracing shader into a texture.
 */
void engine_render_raytraced_scene_to_texture(renderer_engine_t *engine, camera_t *cam)
{
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, engine->render_texture, 0);

    glViewport(0, 0, engine->render_texture_width, engine->render_texture_height);
    glUseProgram(engine->raytracer_shader_program);

    vector3_t pos = camera_get_position(cam);
    vector3_t fwd = vector3_normalize(vector3_subtract(cam->target, pos));
    vector3_t global_up = {0, 1, 0};
    vector3_t right = vector3_normalize(vector3_cross(fwd, global_up));
    vector3_t up = vector3_cross(right, fwd);

    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camPos"), pos.x, pos.y, pos.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camRight"), right.x, right.y, right.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camUp"), up.x, up.y, up.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camForward"), fwd.x, fwd.y, fwd.z);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "tanHalfFov"), tanf(M_PI / 6.0f));
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "aspect"), (float)engine->window_width / (float)engine->window_height);
    glUniform1i(glGetUniformLocation(engine->raytracer_shader_program, "moving"), cam->is_moving ? 1 : 0);
    glUniform2f(glGetUniformLocation(engine->raytracer_shader_program, "resolution"),
                (float)engine->render_texture_width, (float)engine->render_texture_height);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "time"), (float)glfwGetTime());

    float disk_inner_radius = BLACK_HOLE_SCHWARZSCHILD_RADIUS * 2.2f;
    float disk_outer_radius = BLACK_HOLE_SCHWARZSCHILD_RADIUS * 5.2f;
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "disk_r1"), disk_inner_radius);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "disk_r2"), disk_outer_radius);

    glUniform1i(glGetUniformLocation(engine->raytracer_shader_program, "numObjects"), NUM_CELESTIAL_BODIES);
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        char uniform_name[64];
        snprintf(uniform_name, sizeof(uniform_name), "objPosRadius[%d]", i);
        glUniform4fv(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), 1, &celestial_bodies[i].position_and_radius.x);
        snprintf(uniform_name, sizeof(uniform_name), "objColor[%d]", i);
        glUniform4fv(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), 1, &celestial_bodies[i].color.x);
        snprintf(uniform_name, sizeof(uniform_name), "objMass[%d]", i);
        glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), celestial_bodies[i].mass);
    }
    
    glBindVertexArray(engine->fullscreen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
}

/**
 * @brief Renders the previously generated texture to the screen.
 */
void engine_render_texture_to_screen(renderer_engine_t *engine)
{
    glViewport(0, 0, engine->window_width, engine->window_height);

    glUseProgram(engine->texture_quad_shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, engine->render_texture);
    glUniform1i(glGetUniformLocation(engine->texture_quad_shader_program, "screenTexture"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    glBindVertexArray(engine->fullscreen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

// GLFW CALLBACKS

void callback_mouse_button(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            camera.is_dragging_orbit = true;
            glfwGetCursorPos(window, &camera.last_cursor_x, &camera.last_cursor_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.is_dragging_orbit = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            camera.is_dragging_pan = true;
            glfwGetCursorPos(window, &camera.last_cursor_x, &camera.last_cursor_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.is_dragging_pan = false;
        }
    }
    camera_update_moving_state(&camera);
}

void callback_cursor_position(GLFWwindow *window, double xpos, double ypos)
{
    camera_process_mouse_move(&camera, xpos, ypos);
}

void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    camera_process_scroll(&camera, yoffset);
}

void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_R:
            camera_reset(&camera);
            printf("[INFO] Camera reset\n");
            break;
        case GLFW_KEY_P:
            is_physics_paused = !is_physics_paused;
            printf("[INFO] Physics %s\n", is_physics_paused ? "paused" : "resumed");
            break;
        case GLFW_KEY_G:
            is_grid_visible = !is_grid_visible;
            printf("[INFO] Grid %s\n", is_grid_visible ? "visible" : "hidden");
            break;
        }
    }
}

void callback_framebuffer_size(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    renderer_engine.window_width = width;
    renderer_engine.window_height = height;

    renderer_engine.render_texture_width = width;
    renderer_engine.render_texture_height = height;
    glBindTexture(GL_TEXTURE_2D, renderer_engine.render_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderer_engine.render_texture_width, renderer_engine.render_texture_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// APPLICATION LIFECYCLE

/**
 * @brief Initializes GLFW, OpenGL context, shaders, and all rendering objects.
 * @return True on success, false on failure.
 */
bool engine_initialize(renderer_engine_t *engine)
{
    if (!glfwInit())
    {
        printf("Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    engine->window = glfwCreateWindow(500, 300, "Black Hole", NULL, NULL);
    if (!engine->window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(engine->window);
    glfwSetFramebufferSizeCallback(engine->window, callback_framebuffer_size);
    glfwGetFramebufferSize(engine->window, &engine->window_width, &engine->window_height);
    glViewport(0, 0, engine->window_width, engine->window_height);

    engine->render_texture_width = engine->window_width / 7; // low resolution to improve performance
    engine->render_texture_height = engine->window_height / 7;

    glfwSwapInterval(1); // v-sync

    printf("--- Black Hole ---\n");
    printf("Initial Framebuffer Size: %d x %d pixels\n", engine->window_width, engine->window_height);
    printf("Compute Resolution: %d x %d pixels\n", engine->render_texture_width, engine->render_texture_height);
    printf("--- CONTROLS ---\n");
    printf("Left Mouse + Drag: Orbit Camera\n");
    printf("Middle Mouse + Drag: Pan Camera\n");
    printf("Mouse Wheel: Zoom\n");
    printf("R: Reset Camera\n");
    printf("P: Pause/Resume Physics\n");
    printf("G: Toggle Spacetime Grid\n");
    printf("ESC: Exit\n");
    printf("----------------\n");

    engine->raytracer_shader_program = utility_create_shader_program(quad_vertex_shader_source, raytracer_fragment_shader_source);
    engine->grid_shader_program = utility_create_shader_program(grid_vertex_shader_source, grid_fragment_shader_source);
    engine->texture_quad_shader_program = utility_create_shader_program(quad_vertex_shader_source, quad_fragment_shader_source);

    if (!engine->raytracer_shader_program || !engine->grid_shader_program || !engine->texture_quad_shader_program)
    {
        return false;
    }

    engine_init_fullscreen_quad(engine);
    engine_init_render_texture(engine);

    glfwSetMouseButtonCallback(engine->window, callback_mouse_button);
    glfwSetCursorPosCallback(engine->window, callback_cursor_position);
    glfwSetScrollCallback(engine->window, callback_scroll);
    glfwSetKeyCallback(engine->window, callback_key);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

/**
 * @brief Cleans up all allocated resources.
 */
void engine_cleanup(renderer_engine_t *engine)
{
    if (engine->fullscreen_quad_vao) glDeleteVertexArrays(1, &engine->fullscreen_quad_vao);
    if (engine->render_texture) glDeleteTextures(1, &engine->render_texture);
    if (engine->raytracer_shader_program) glDeleteProgram(engine->raytracer_shader_program);
    if (engine->grid_shader_program) glDeleteProgram(engine->grid_shader_program);
    if (engine->texture_quad_shader_program) glDeleteProgram(engine->texture_quad_shader_program);
    if (engine->grid_vao) glDeleteVertexArrays(1, &engine->grid_vao);
    if (engine->grid_vbo) glDeleteBuffers(1, &engine->grid_vbo);
    if (engine->grid_ebo) glDeleteBuffers(1, &engine->grid_ebo);

    if (engine->window)
    {
        glfwDestroyWindow(engine->window);
    }
    glfwTerminate();
}

/**
 * @brief Main entry point of the application.
 */
int main()
{
    camera_reset(&camera);

    if (!engine_initialize(&renderer_engine))
    {
        return EXIT_FAILURE;
    }

    grid_generate_mesh(&renderer_engine);

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(renderer_engine.window))
    {
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;

        simulation_update_physics(delta_time * 500.0); // speed up simulation time
        if (!is_physics_paused)
        {
            grid_generate_mesh(&renderer_engine);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vector3_t cam_pos = camera_get_position(&camera);
        matrix4_t view_matrix = matrix4_look_at(cam_pos, camera.target, (vector3_t){0, 1, 0});
        matrix4_t projection_matrix = matrix4_perspective(M_PI / 3.0f, (float)renderer_engine.window_width / (float)renderer_engine.window_height, 1e9f, 1e14f);
        matrix4_t view_projection_matrix = matrix4_multiply(projection_matrix, view_matrix);

        grid_render(&renderer_engine, view_projection_matrix);
        engine_render_raytraced_scene_to_texture(&renderer_engine, &camera);
        engine_render_texture_to_screen(&renderer_engine);

        glfwSwapBuffers(renderer_engine.window);
        glfwPollEvents();
    }

    engine_cleanup(&renderer_engine);
    return EXIT_SUCCESS;
}

// SHADER SOURCES

const char *quad_vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *quad_fragment_shader_source =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D screenTexture;\n"
    "void main() {\n"
    "    FragColor = texture(screenTexture, TexCoord);\n"
    "}\n";

const char *grid_vertex_shader_source =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 viewProj;\n"
    "void main() {\n"
    "    gl_Position = viewProj * vec4(aPos, 1.0);\n"
    "}\n";

const char *grid_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
    "}\n";

const char *raytracer_fragment_shader_source =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "\n"
    "// Uniforms\n"
    "uniform vec3 camPos;\n"
    "uniform vec3 camRight;\n"
    "uniform vec3 camUp;\n"
    "uniform vec3 camForward;\n"
    "uniform float tanHalfFov;\n"
    "uniform float aspect;\n"
    "uniform bool moving;\n"
    "uniform float disk_r1;\n"
    "uniform float disk_r2;\n"
    "uniform int numObjects;\n"
    "uniform vec4 objPosRadius[16];\n"
    "uniform vec4 objColor[16];\n"
    "uniform float objMass[16];\n"
    "uniform vec2 resolution;\n"
    "uniform float time;\n"
    "\n"
    "const float blackhole = 1.269e10;\n"
    "float D_LAMBDA = 5e7;\n"
    "const float ESCAPE_R = 1e30;\n"
    "\n"
    "struct Ray {\n"
    "    float x, y, z, r, theta, phi;\n"
    "    float dr, dtheta, dphi;\n"
    "    float E, L;\n"
    "};\n"
    "\n"
    "// Global hit variables\n"
    "vec4 hitObjectColor;\n"
    "vec3 hitCenter;\n"
    "float hitRadius;\n"
    "float random(vec3 p) {\n"
    "    return fract(sin(dot(p, vec3(12.9898, 78.233, 151.7182))) * 43758.5453);\n"
    "}\n"
    "vec4 getStarColor(vec3 dir) {\n"
    "    float star_density = 0.9995;\n"
    "    float r = random(dir);\n"
    "    if (r > star_density) {\n"
    "        float star_brightness = (r - star_density) / (1.0 - star_density);\n"
    "        return vec4(vec3(star_brightness), 1.0);\n"
    "    }\n"
    "    return vec4(0.0);\n"
    "}\n"
    "\n"
    "Ray initRay(vec3 pos, vec3 dir) {\n"
    "    Ray ray;\n"
    "    ray.x = pos.x; ray.y = pos.y; ray.z = pos.z;\n"
    "    ray.r = length(pos);\n"
    "    ray.theta = acos(pos.z / ray.r);\n"
    "    ray.phi = atan(pos.y, pos.x);\n"
    "\n"
    "    float dx = dir.x, dy = dir.y, dz = dir.z;\n"
    "    ray.dr = sin(ray.theta)*cos(ray.phi)*dx + sin(ray.theta)*sin(ray.phi)*dy + cos(ray.theta)*dz;\n"
    "    ray.dtheta = (cos(ray.theta)*cos(ray.phi)*dx + cos(ray.theta)*sin(ray.phi)*dy - sin(ray.theta)*dz) / ray.r;\n"
    "    ray.dphi = (-sin(ray.phi)*dx + cos(ray.phi)*dy) / (ray.r * sin(ray.theta));\n"
    "\n"
    "    ray.L = ray.r * ray.r * sin(ray.theta) * ray.dphi;\n"
    "    float f = 1.0 - blackhole / ray.r;\n"
    "    float dt_dL = sqrt((ray.dr*ray.dr)/f + ray.r*ray.r*(ray.dtheta*ray.dtheta + sin(ray.theta)*sin(ray.theta)*ray.dphi*ray.dphi));\n"
    "    ray.E = f * dt_dL;\n"
    "\n"
    "    return ray;\n"
    "}\n"
    "\n"
    "bool intercept(Ray ray, float rs) {\n"
    "    return ray.r <= rs;\n"
    "}\n"
    "\n"
    "bool interceptObject(Ray ray) {\n"
    "    vec3 P = vec3(ray.x, ray.y, ray.z);\n"
    "    for (int i = 0; i < numObjects; ++i) {\n"
    "        vec3 center = objPosRadius[i].xyz;\n"
    "        float radius = objPosRadius[i].w;\n"
    "        if (distance(P, center) <= radius) {\n"
    "            hitObjectColor = objColor[i];\n"
    "            hitCenter = center;\n"
    "            hitRadius = radius;\n"
    "            return true;\n"
    "        }\n"
    "    }\n"
    "    return false;\n"
    "}\n"
    "\n"
    "void geodesicRHS(Ray ray, out vec3 d1, out vec3 d2) {\n"
    "    float r = ray.r, theta = ray.theta;\n"
    "    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;\n"
    "    float f = 1.0 - blackhole / r;\n"
    "    float dt_dL = ray.E / f;\n"
    "\n"
    "    d1 = vec3(dr, dtheta, dphi);\n"
    "    d2.x = -(blackhole / (2.0 * r*r)) * f * dt_dL * dt_dL\n"
    "         + (blackhole / (2.0 * r*r * f)) * dr * dr\n"
    "         + r * (dtheta*dtheta + sin(theta)*sin(theta)*dphi*dphi);\n"
    "    d2.y = -2.0*dr*dtheta/r + sin(theta)*cos(theta)*dphi*dphi;\n"
    "    d2.z = -2.0*dr*dphi/r - 2.0*cos(theta)/(sin(theta)) * dtheta * dphi;\n"
    "}\n"
    "\n"
    "void rk4Step(inout Ray ray, float dL) {\n"
    "    vec3 k1a, k1b;\n"
    "    geodesicRHS(ray, k1a, k1b);\n"
    "    \n"
    "    ray.r      += dL * k1a.x;\n"
    "    ray.theta  += dL * k1a.y;\n"
    "    ray.phi    += dL * k1a.z;\n"
    "    ray.dr     += dL * k1b.x;\n"
    "    ray.dtheta += dL * k1b.y;\n"
    "    ray.dphi   += dL * k1b.z;\n"
    "\n"
    "    ray.x = ray.r * sin(ray.theta) * cos(ray.phi);\n"
    "    ray.y = ray.r * sin(ray.theta) * sin(ray.phi);\n"
    "    ray.z = ray.r * cos(ray.theta);\n"
    "}\n"
    "\n"
    "bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {\n"
    "    bool crossed = (oldPos.y * newPos.y < 0.0);\n"
    "    float r = length(vec2(newPos.x, newPos.z));\n"
    "    return crossed && (r >= disk_r1 && r <= disk_r2);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 pix = gl_FragCoord.xy;\n"
    "\n"
    "    float u = (2.0 * (pix.x + 0.5) / resolution.x - 1.0) * aspect * tanHalfFov;\n"
    "    float v = (1.0 - 2.0 * (pix.y + 0.5) / resolution.y) * tanHalfFov;\n"
    "    vec3 dir = normalize(u * camRight - v * camUp + camForward);\n"
    "    Ray ray = initRay(camPos, dir);\n"
    "\n"
    "    vec4 color = vec4(0.0);\n"
    "    vec3 prevPos = vec3(ray.x, ray.y, ray.z);\n"
    "\n"
    "    bool hitBlackHole = false;\n"
    "    bool hitDisk = false;\n"
    "    bool hitObject = false;\n"
    "\n"
    "    int steps = moving ? 25000 : 26000;\n"
    "\n"
    "    for (int i = 0; i < steps; ++i) {\n"
    "        if (intercept(ray, blackhole)) { hitBlackHole = true; break; }\n"
    "        float step_scale = clamp(ray.r / (blackhole * 20.0), 0.1, 5.0);\n"
    "        float dynamic_step = D_LAMBDA * step_scale;\n"
    "        rk4Step(ray, dynamic_step);\n"
    "        vec3 newPos = vec3(ray.x, ray.y, ray.z);\n"
    "        if (crossesEquatorialPlane(prevPos, newPos)) { hitDisk = true; break; }\n"
    "        if (interceptObject(ray)) { hitObject = true; break; }\n"
    "        prevPos = newPos;\n"
    "        if (ray.r > ESCAPE_R) break;\n"
    "    }\n"
    "    if (hitDisk) {\n"
    "        vec3 hitPos = vec3(ray.x, ray.y, ray.z);\n"
    "        float r_norm = (length(hitPos) - disk_r1) / (disk_r2 - disk_r1);\n"
    "        r_norm = clamp(r_norm, 0.0, 1.0);\n"
    "        \n"
    "        vec3 color_hot = vec3(1.0, 1.0, 0.8);\n"
    "        vec3 color_mid = vec3(1.0, 0.5, 0.0);\n"
    "        vec3 color_cool = vec3(0.8, 0.0, 0.0);\n"
    "        \n"
    "        vec3 diskColor = mix(color_mid, color_hot, smoothstep(0.0, 0.3, 1.0 - r_norm));\n"
    "        diskColor = mix(color_cool, diskColor, smoothstep(0.3, 1.0, 1.0 - r_norm));\n"
    "        float angle = atan(hitPos.y, hitPos.x);\n"
    "        float spiral = 0.5 + 0.5 * sin(angle * 10.0 - r_norm * 20.0 - time * 0.1);\n"
    "        diskColor *= 0.8 + 0.4 * spiral;\n"
    "        \n"
    "        color = vec4(diskColor, 1.0);\n"
    "    } else if (hitBlackHole) {\n"
    "        color = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    } else if (hitObject) {\n"
    "        vec3 P = vec3(ray.x, ray.y, ray.z);\n"
    "        vec3 N = normalize(P - hitCenter);\n"
    "        vec3 V = normalize(camPos - P);\n"
    "        vec3 L = normalize(vec3(-1, 1, -1));\n"
    "\n"
    "        float ambient = 0.1;\n"
    "        float diff = max(dot(N, L), 0.0);\n"
    "        vec3 shaded = hitObjectColor.rgb * (ambient + diff);\n"
    "\n"
    "        vec3 H = normalize(L + V);\n"
    "        float spec = pow(max(dot(N, H), 0.0), 32.0);\n"
    "        vec3 specular = vec3(1.0, 1.0, 1.0) * spec * 0.5;\n"
    "\n"
    "        color = vec4(shaded + specular, hitObjectColor.a);\n"
    "    } else {\n"
    "        color = getStarColor(dir);\n"
    "    }\n"
    "\n"
    "    FragColor = color;\n"
    "}\n";