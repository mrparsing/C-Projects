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

static const double c_speed = 299792458.0;
static const double G_const = 6.67430e-11;
static const float blackhole = 1.269e10f;
static float D_LAMBDA = 5e7f;
static const double ESCAPE_R = 1e30;
static double last_print_time = 0.0;
static int frames_count = 0;
static bool physics_paused = false;
static bool grid_visible = true;
static const int num_objects = 3;

typedef struct
{
    float x, y, z;
} vec3_t;

typedef struct
{
    float x, y, z, w;
} vec4_t;

typedef struct
{
    float m[16];
} mat4_t;

typedef struct
{
    vec3_t target;
    float radius;
    float min_radius, max_radius;
    float azimuth;
    float elevation;
    float orbit_speed;
    float pan_speed;
    double zoom_speed;
    bool dragging;
    bool panning;
    bool moving;
    double last_x, last_y;
} camera_t;

typedef struct
{
    vec4_t pos_radius;
    vec4_t color;
    float mass;
    vec3_t velocity;
} object;

typedef struct
{
    GLFWwindow *window;
    GLuint quad_vao;
    GLuint texture;
    GLuint raytracer_program;
    GLuint grid_shader_program;
    GLuint quad_program;
    GLuint grid_vao, grid_vbo, grid_ebo;
    int grid_index_count;
    int width, height;
    int compute_width, compute_height;
} engine_t;

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
    .dragging = false,
    .panning = false,
    .moving = false,
    .last_x = 0.0,
    .last_y = 0.0};
static camera_t camera;

static object objects[] = {
    {{2.3e11f, 0.0f, 0.0f, 4e10f},
     {0.4, 0.7, 1.0, 1.0},
     1.98892e30f,
     {0.0f, 0.0f, 5.34e7}},
    {{-1.6e11f, 0.0f, 0.0f, 4e10f},
     {0.8, 0.3, 0.2, 1.0},
     1.98892e30f,
     {0.0f, 0.0f, -5.34e7}},
    {{0.0f, 0.0f, 0.0f, blackhole},
     {0, 0, 0, 1},
     8.54e36f,
     {0, 0, 0}}};

static engine_t engine;

float vec3_length(vec3_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalize(vec3_t v)
{
    float len = vec3_length(v);
    if (len > 0.0f)
    {
        return (vec3_t){v.x / len, v.y / len, v.z / len};
    }
    return (vec3_t){0.0f, 0.0f, 0.0f};
}

vec3_t vec3_cross(vec3_t a, vec3_t b)
{
    return (vec3_t){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3_t vec3_add(vec3_t a, vec3_t b)
{
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3_t vec3_scale(vec3_t v, float s)
{
    return (vec3_t){v.x * s, v.y * s, v.z * s};
}

float clamp_float(float value, float min_val, float max_val)
{
    if (value < min_val)
        return min_val;
    if (value > max_val)
        return max_val;
    return value;
}

vec3_t camera_position(const camera_t *cam)
{
    float clamped_elevation = clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    vec3_t orbital_pos = {
        cam->radius * sinf(clamped_elevation) * cosf(cam->azimuth),
        cam->radius * cosf(clamped_elevation),
        cam->radius * sinf(clamped_elevation) * sinf(cam->azimuth)};
    return vec3_add(cam->target, orbital_pos);
}

void camera_update(camera_t *cam)
{
    cam->moving = cam->dragging || cam->panning;
}

void camera_process_mouse_move(camera_t *cam, double x, double y)
{
    float dx = (float)(x - cam->last_x);
    float dy = (float)(y - cam->last_y);

    if (cam->dragging)
    {
        cam->azimuth += dx * cam->orbit_speed;
        cam->elevation -= dy * cam->orbit_speed;
        cam->elevation = clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    }

    if (cam->panning)
    {
        vec3_t pos = camera_position(cam);
        vec3_t fwd = vec3_normalize(vec3_sub(cam->target, pos));
        vec3_t global_up = {0, 1, 0};
        vec3_t right = vec3_normalize(vec3_cross(fwd, global_up));
        vec3_t up = vec3_normalize(vec3_cross(right, fwd));

        float pan_scale = cam->radius / 1e11f;
        vec3_t pan_offset_x = vec3_scale(right, -dx * cam->pan_speed * pan_scale);
        vec3_t pan_offset_y = vec3_scale(up, dy * cam->pan_speed * pan_scale);
        cam->target = vec3_add(cam->target, pan_offset_x);
        cam->target = vec3_add(cam->target, pan_offset_y);
    }

    cam->last_x = x;
    cam->last_y = y;
    camera_update(cam);
}

void camera_process_scroll(camera_t *cam, double yoffset)
{
    cam->radius -= (float)(yoffset * cam->zoom_speed);
    cam->radius = clamp_float(cam->radius, cam->min_radius, cam->max_radius);
    camera_update(cam);
}

void reset_camera(camera_t *cam)
{
    *cam = initial_camera_state;
}

mat4_t mat4_identity()
{
    mat4_t m = {0};
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

mat4_t mat4_perspective(float fovy, float aspect, float znear, float zfar)
{
    mat4_t result = {0};
    float tanHalfFovy = tanf(fovy / 2.0f);
    result.m[0] = 1.0f / (aspect * tanHalfFovy);
    result.m[5] = 1.0f / tanHalfFovy;
    result.m[10] = -(zfar + znear) / (zfar - znear);
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * zfar * znear) / (zfar - znear);
    return result;
}

mat4_t mat4_lookat(vec3_t eye, vec3_t center, vec3_t up)
{
    vec3_t f = vec3_normalize(vec3_sub(center, eye));
    vec3_t s = vec3_normalize(vec3_cross(f, up));
    vec3_t u = vec3_cross(s, f);
    mat4_t result = mat4_identity();
    result.m[0] = s.x;
    result.m[4] = s.y;
    result.m[8] = s.z;
    result.m[1] = u.x;
    result.m[5] = u.y;
    result.m[9] = u.z;
    result.m[2] = -f.x;
    result.m[6] = -f.y;
    result.m[10] = -f.z;
    result.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    result.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    result.m[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
    return result;
}

mat4_t mat4_multiply(mat4_t a, mat4_t b)
{
    mat4_t result = {0};
    for (int c = 0; c < 4; ++c)
    {
        for (int r = 0; r < 4; ++r)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                sum += a.m[k * 4 + r] * b.m[c * 4 + k];
            }
            result.m[c * 4 + r] = sum;
        }
    }
    return result;
}

GLuint compile_shader(const char *source, GLenum type)
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

GLuint create_shader_program(const char *vertex_source, const char *fragment_source)
{
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);

    if (!vertex_shader || !fragment_shader)
    {
        return 0;
    }

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

// Shader sources
const char *quad_vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *quad_fragment_shader =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D screenTexture;\n"
    "void main() {\n"
    "    FragColor = texture(screenTexture, TexCoord);\n"
    "}\n";

const char *grid_vertex_shader =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 viewProj;\n"
    "void main() {\n"
    "    gl_Position = viewProj * vec4(aPos, 1.0);\n"
    "}\n";

const char *grid_fragment_shader =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
    "}\n";

const char *raytracer_fragment_shader =
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
    "        vec3 L = normalize(vec3(-1, 1, -1)); // Direzione luce fissa\n"
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

// Initialize OpenGL objects
void init_quad_vao(engine_t *eng)
{
    float quad_vertices[] = {
        // positions   // texCoords
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    GLuint vbo;
    glGenVertexArrays(1, &eng->quad_vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(eng->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void init_texture(engine_t *eng)
{
    glGenTextures(1, &eng->texture);
    glBindTexture(GL_TEXTURE_2D, eng->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, eng->compute_width, eng->compute_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void generate_grid(engine_t *eng)
{
    const int grid_size = 50;
    const float spacing = 1e10f;
    const double planet_curvature_scale = 500.0;

    vec3_t *vertices = malloc((grid_size + 1) * (grid_size + 1) * sizeof(vec3_t));
    GLuint *indices = malloc(grid_size * grid_size * 4 * sizeof(GLuint));

    int vertex_count = 0;
    int index_count = 0;

    for (int z = 0; z <= grid_size; ++z)
    {
        for (int x = 0; x <= grid_size; ++x)
        {
            float world_x = (x - grid_size / 2) * spacing;
            float world_z = (z - grid_size / 2) * spacing;
            float y = -25e10f;

            for (int i = 0; i < num_objects; ++i)
            {
                vec3_t obj_pos = {objects[i].pos_radius.x, objects[i].pos_radius.y, objects[i].pos_radius.z};
                double mass = objects[i].mass;
                double r_s = 2.0 * G_const * mass / (c_speed * c_speed);
                double dx = world_x - obj_pos.x;
                double dz = world_z - obj_pos.z;
                double dist_sq = dx * dx + dz * dz;

                double delta_y = 0.0;
                if (dist_sq > r_s * r_s)
                {
                    double dist = sqrt(dist_sq);
                    delta_y = sqrt(8.0 * r_s * (dist - r_s));
                    if (i != 2)
                    {
                        delta_y *= planet_curvature_scale;
                    }
                }
                y += (float)delta_y;
            }
            vertices[vertex_count++] = (vec3_t){world_x, y, world_z};
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

    if (eng->grid_vao == 0)
    {
        glGenVertexArrays(1, &eng->grid_vao);
        glGenBuffers(1, &eng->grid_vbo);
        glGenBuffers(1, &eng->grid_ebo);
    }

    glBindVertexArray(eng->grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, eng->grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vec3_t), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eng->grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3_t), (void *)0);
    eng->grid_index_count = index_count;
    glBindVertexArray(0);
    free(vertices);
    free(indices);
}

void draw_grid(engine_t *eng, mat4_t view_proj)
{
    if (!grid_visible)
        return;

    glUseProgram(eng->grid_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(eng->grid_shader_program, "viewProj"),
                       1, GL_FALSE, view_proj.m);
    glBindVertexArray(eng->grid_vao);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_LINES, eng->grid_index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void render_raytracer(engine_t *eng, camera_t *cam)
{
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eng->texture, 0);

    glViewport(0, 0, eng->compute_width, eng->compute_height);
    glUseProgram(eng->raytracer_program);

    vec3_t pos = camera_position(cam);
    vec3_t fwd = vec3_normalize(vec3_sub(cam->target, pos));
    vec3_t up = {0, 1, 0};
    vec3_t right = vec3_normalize(vec3_cross(fwd, up));
    up = vec3_cross(right, fwd);

    glUniform3f(glGetUniformLocation(eng->raytracer_program, "camPos"), pos.x, pos.y, pos.z);
    glUniform3f(glGetUniformLocation(eng->raytracer_program, "camRight"), right.x, right.y, right.z);
    glUniform3f(glGetUniformLocation(eng->raytracer_program, "camUp"), up.x, up.y, up.z);
    glUniform3f(glGetUniformLocation(eng->raytracer_program, "camForward"), fwd.x, fwd.y, fwd.z);
    glUniform1f(glGetUniformLocation(eng->raytracer_program, "tanHalfFov"), tanf(M_PI / 6.0f));
    glUniform1f(glGetUniformLocation(eng->raytracer_program, "aspect"), (float)eng->width / (float)eng->height);
    glUniform1i(glGetUniformLocation(eng->raytracer_program, "moving"), cam->moving ? 1 : 0);
    glUniform2f(glGetUniformLocation(eng->raytracer_program, "resolution"),
                (float)eng->compute_width, (float)eng->compute_height);
    glUniform1f(glGetUniformLocation(eng->raytracer_program, "time"), (float)glfwGetTime());

    float r1 = blackhole * 2.2f;
    float r2 = blackhole * 5.2f;
    glUniform1f(glGetUniformLocation(eng->raytracer_program, "disk_r1"), r1);
    glUniform1f(glGetUniformLocation(eng->raytracer_program, "disk_r2"), r2);

    glUniform1i(glGetUniformLocation(eng->raytracer_program, "numObjects"), num_objects);
    for (int i = 0; i < num_objects; ++i)
    {
        char uniform_name[64];
        snprintf(uniform_name, sizeof(uniform_name), "objPosRadius[%d]", i);
        GLint pos_radius_loc = glGetUniformLocation(eng->raytracer_program, uniform_name);
        glUniform4f(pos_radius_loc, objects[i].pos_radius.x, objects[i].pos_radius.y,
                    objects[i].pos_radius.z, objects[i].pos_radius.w);
        snprintf(uniform_name, sizeof(uniform_name), "objColor[%d]", i);
        GLint color_loc = glGetUniformLocation(eng->raytracer_program, uniform_name);
        glUniform4f(color_loc, objects[i].color.x, objects[i].color.y,
                    objects[i].color.z, objects[i].color.w);
        snprintf(uniform_name, sizeof(uniform_name), "objMass[%d]", i);
        GLint mass_loc = glGetUniformLocation(eng->raytracer_program, uniform_name);
        glUniform1f(mass_loc, objects[i].mass);
    }

    glBindVertexArray(eng->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
}

void draw_fullscreen_quad(engine_t *eng)
{
    glViewport(0, 0, eng->width, eng->height);

    glUseProgram(eng->quad_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, eng->texture);
    glUniform1i(glGetUniformLocation(eng->quad_program, "screenTexture"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(eng->quad_vao);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void update_physics(double dt)
{
    if (physics_paused)
        return;

    for (int i = 0; i < num_objects; ++i)
    {
        for (int j = 0; j < num_objects; ++j)
        {
            if (i == j)
                continue;

            float dx = objects[j].pos_radius.x - objects[i].pos_radius.x;
            float dy = objects[j].pos_radius.y - objects[i].pos_radius.y;
            float dz = objects[j].pos_radius.z - objects[i].pos_radius.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            if (distance > (objects[i].pos_radius.w + objects[j].pos_radius.w))
            {
                vec3_t direction = {dx / distance, dy / distance, dz / distance};
                double g_force = (G_const * objects[i].mass * objects[j].mass) / (distance * distance);
                double acc = g_force / objects[i].mass;

                objects[i].velocity.x += direction.x * acc * dt;
                objects[i].velocity.y += direction.y * acc * dt;
                objects[i].velocity.z += direction.z * acc * dt;
            }
        }
    }
    for (int i = 0; i < num_objects; i++)
    {
        objects[i].pos_radius.x += objects[i].velocity.x * dt;
        objects[i].pos_radius.y += objects[i].velocity.y * dt;
        objects[i].pos_radius.z += objects[i].velocity.z * dt;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            camera.dragging = true;
            glfwGetCursorPos(window, &camera.last_x, &camera.last_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.dragging = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            camera.panning = true;
            glfwGetCursorPos(window, &camera.last_x, &camera.last_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.panning = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    camera_process_mouse_move(&camera, xpos, ypos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera_process_scroll(&camera, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_R:
            reset_camera(&camera);
            printf("[INFO] Camera reset\n");
            break;
        case GLFW_KEY_P:
            physics_paused = !physics_paused;
            printf("[INFO] Physics %s\n", physics_paused ? "paused" : "resumed");
            break;
        case GLFW_KEY_G:
            grid_visible = !grid_visible;
            printf("[INFO] Grid %s\n", grid_visible ? "visible" : "hidden");
            break;
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    engine.width = width;
    engine.height = height;

    engine.compute_width = width;
    engine.compute_height = height;
    glBindTexture(GL_TEXTURE_2D, engine.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, engine.compute_width, engine.compute_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool init_engine(engine_t *eng)
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

    eng->window = glfwCreateWindow(500, 300, "Black Hole", NULL, NULL);
    if (!eng->window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(eng->window);
    glfwSetFramebufferSizeCallback(eng->window, framebuffer_size_callback);
    glfwGetFramebufferSize(eng->window, &eng->width, &eng->height);
    glViewport(0, 0, eng->width, eng->height);

    eng->compute_width = eng->width / 8;
    eng->compute_height = eng->height / 8;

    glfwSwapInterval(1);

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Initial Framebuffer Size: %d x %d pixels\n", eng->width, eng->height);
    printf("Compute Resolution: %d x %d pixels\n", eng->compute_width, eng->compute_height);
    printf("--- CONTROLS ---\n");
    printf("Left Mouse + Drag: Orbit Camera\n");
    printf("Middle Mouse + Drag: Pan Camera\n");
    printf("Mouse Wheel: Zoom\n");
    printf("R: Reset Camera\n");
    printf("P: Pause/Resume Physics\n");
    printf("G: Toggle Spacetime Grid\n");
    printf("ESC: Exit\n");
    printf("----------------\n");

    eng->raytracer_program = create_shader_program(quad_vertex_shader, raytracer_fragment_shader);
    eng->grid_shader_program = create_shader_program(grid_vertex_shader, grid_fragment_shader);
    eng->quad_program = create_shader_program(quad_vertex_shader, quad_fragment_shader);

    if (!eng->raytracer_program || !eng->grid_shader_program || !eng->quad_program)
    {
        return false;
    }

    init_quad_vao(eng);
    init_texture(eng);

    glfwSetMouseButtonCallback(eng->window, mouse_button_callback);
    glfwSetCursorPosCallback(eng->window, cursor_position_callback);
    glfwSetScrollCallback(eng->window, scroll_callback);
    glfwSetKeyCallback(eng->window, key_callback);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

void cleanup_engine(engine_t *eng)
{
    if (eng->quad_vao)
        glDeleteVertexArrays(1, &eng->quad_vao);
    if (eng->texture)
        glDeleteTextures(1, &eng->texture);
    if (eng->raytracer_program)
        glDeleteProgram(eng->raytracer_program);
    if (eng->grid_shader_program)
        glDeleteProgram(eng->grid_shader_program);
    if (eng->quad_program)
        glDeleteProgram(eng->quad_program);
    if (eng->grid_vao)
        glDeleteVertexArrays(1, &eng->grid_vao);
    if (eng->grid_vbo)
        glDeleteBuffers(1, &eng->grid_vbo);
    if (eng->grid_ebo)
        glDeleteBuffers(1, &eng->grid_ebo);

    if (eng->window)
    {
        glfwDestroyWindow(eng->window);
    }
    glfwTerminate();
}

int main()
{
    reset_camera(&camera);

    if (!init_engine(&engine))
    {
        return EXIT_FAILURE;
    }

    generate_grid(&engine);

    double last_time = glfwGetTime();
    double last_pos_print_time = 0.0;

    while (!glfwWindowShouldClose(engine.window))
    {
        double current_time = glfwGetTime();
        double dt = current_time - last_time;
        last_time = current_time;

        if (current_time - last_pos_print_time >= 1.0)
        {
            vec3_t current_cam_pos = camera_position(&camera);

            last_pos_print_time = current_time;
        }
        update_physics(dt * 500.0);

        if (!physics_paused)
        {
            generate_grid(&engine);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vec3_t cam_pos = camera_position(&camera);
        mat4_t view = mat4_lookat(cam_pos, camera.target, (vec3_t){0, 1, 0});
        mat4_t proj = mat4_perspective(M_PI / 3.0f, (float)engine.width / (float)engine.height, 1e9f, 1e14f);
        mat4_t view_proj = mat4_multiply(proj, view);

        draw_grid(&engine, view_proj);
        render_raytracer(&engine, &camera);
        draw_fullscreen_quad(&engine);

        glfwSwapBuffers(engine.window);
        glfwPollEvents();

        frames_count++;
        if (current_time - last_print_time >= 1.0)
        {
            // printf("FPS: %d\n", frames_count);
            frames_count = 0;
            last_print_time = current_time;
        }
    }

    cleanup_engine(&engine);
    return EXIT_SUCCESS;
}
