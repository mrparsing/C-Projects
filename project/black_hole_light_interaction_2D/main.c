#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

int fbWidth = WINDOW_WIDTH;
int fbHeight = WINDOW_HEIGHT;

// background shader
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec2 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoord;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main() {\n"
                                 "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
                                 "   TexCoord = aTexCoord;\n"
                                 "}\n";

const char *sceneFragmentShaderSource = "#version 330 core\n"
                                        "in vec2 TexCoord;\n"
                                        "out vec4 FragColor;\n"
                                        "uniform sampler2D backgroundTexture;\n"
                                        "uniform vec2 starPos;\n"
                                        "uniform float starRadius;\n"
                                        "void main() {\n"
                                        "   FragColor = texture(backgroundTexture, TexCoord);\n"
                                        "   vec2 dist = TexCoord - starPos;\n"
                                        "   float d = length(dist);\n"
                                        "   if (d < starRadius) {\n"
                                        "       FragColor = vec4(1.0, 1.0, 0.8, 1.0);\n"
                                        "   } else if (d < starRadius * 2.0) {\n"
                                        "       float intensity = (starRadius * 2.0 - d) / starRadius;\n"
                                        "       FragColor += vec4(1.0, 1.0, 0.8, 1.0) * intensity * 0.3;\n"
                                        "   }\n"
                                        "}\n";

// gravitational lens
const char *distortionFragmentShaderSource = "#version 330 core\n"
                                             "in vec2 TexCoord;\n"
                                             "out vec4 FragColor;\n"
                                             "uniform vec2 blackHolePos;\n"
                                             "uniform float schwarzschildRadius;\n"
                                             "uniform sampler2D sceneTexture;\n"
                                             "void main() {\n"
                                             "   vec2 pos = TexCoord - blackHolePos;\n"
                                             "   float r = length(pos);\n"
                                             "   float rs = schwarzschildRadius;\n"
                                             "   if (r < rs) {\n"                             // Schwarzschild
                                             "       FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n" // center
                                             "   } else if (r < rs * 1.5) {\n"                // ring
                                             "       float intensity = 1.0 - (r - rs) / (0.5 * rs);\n"
                                             "       FragColor = vec4(1.0, 0.8, 0.4, 1.0) * intensity;\n"
                                             "   } else {\n"
                                             "       float deflection = rs / (r * r); // Approssimazione della deflessione gravitazionale\n"
                                             "       vec2 distortedCoord = TexCoord - deflection * pos;\n"
                                             "       distortedCoord = clamp(distortedCoord, 0.0, 1.0);\n"
                                             "       FragColor = texture(sceneTexture, distortedCoord);\n"
                                             "   }\n"
                                             "}\n";


unsigned int createBackgroundTexture()
{
    int width = 512, height = 512;
    unsigned char *data = (unsigned char *)malloc(width * height * 3);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 3;
            data[index] = 0;
            data[index + 1] = 0;
            data[index + 2] = 0; // black
        }
    }
    // random star
    srand((unsigned int)time(NULL));
    int numStars = 200;
    for (int i = 0; i < numStars; i++)
    {
        int x = rand() % width;
        int y = rand() % height;
        int index = (y * width + x) * 3;
        data[index] = 255;
        data[index + 1] = 255;
        data[index + 2] = 255; // white
        if (rand() % 5 == 0)
        {
            if (x + 1 < width)
            {
                int index2 = (y * width + x + 1) * 3;
                data[index2] = 255;
                data[index2 + 1] = 255;
                data[index2 + 2] = 255;
            }
            if (y + 1 < height)
            {
                int index3 = ((y + 1) * width + x) * 3;
                data[index3] = 255;
                data[index3 + 1] = 255;
                data[index3 + 2] = 255;
            }
        }
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    free(data);
    return texture;
}

unsigned int compileShader(const char *source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
    return shader;
}

unsigned int createShaderProgram(const char *fragmentSource)
{
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Linking error: %s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

int main()
{
    if (!glfwInit())
    {
        printf("Errore inizializzazione GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Black Hole simulation", NULL, NULL);
    if (!window)
    {
        printf("Error window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (glewInit() != GLEW_OK)
    {
        printf("Error GLEW\n");
        glfwTerminate();
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f};
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int bgTexture = createBackgroundTexture();

    unsigned int sceneShaderProgram = createShaderProgram(sceneFragmentShaderSource);
    unsigned int distortionShaderProgram = createShaderProgram(distortionFragmentShaderSource);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int fboTexture;
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Error: Framebuffer!\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int sceneStarPosLoc = glGetUniformLocation(sceneShaderProgram, "starPos");
    int sceneStarRadiusLoc = glGetUniformLocation(sceneShaderProgram, "starRadius");

    int blackHolePosLoc = glGetUniformLocation(distortionShaderProgram, "blackHolePos");
    int schwarzschildRadiusLoc = glGetUniformLocation(distortionShaderProgram, "schwarzschildRadius");

    float star_x = 0.2f;
    float star_y = 0.1f;
    float star_radius = 0.03f; // big star radius
    float move_speed = 0.005f;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            star_y += move_speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            star_y -= move_speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            star_x -= move_speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            star_x += move_speed;

        star_x = fminf(fmaxf(star_x, 0.0f), 1.0f);
        star_y = fminf(fmaxf(star_y, 0.0f), 1.0f);

        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        float scaleX = (float)fbWidth / (float)winWidth;
        float scaleY = (float)fbHeight / (float)winHeight;

        float bh_x = (float)(mouse_x * scaleX) / fbWidth;
        float bh_y = 1.0f - (float)(mouse_y * scaleY) / fbHeight;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, fbWidth, fbHeight);


        glUseProgram(sceneShaderProgram);
        glUniform2f(sceneStarPosLoc, star_x, star_y);
        glUniform1f(sceneStarRadiusLoc, star_radius);

        glBindTexture(GL_TEXTURE_2D, bgTexture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbWidth, fbHeight);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(distortionShaderProgram);
        glUniform2f(blackHolePosLoc, bh_x, bh_y);
        float rs = 0.05f;
        glUniform1f(schwarzschildRadiusLoc, rs);

        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &bgTexture);
    glDeleteTextures(1, &fboTexture);
    glDeleteFramebuffers(1, &fbo);
    glDeleteProgram(sceneShaderProgram);
    glDeleteProgram(distortionShaderProgram);

    glfwTerminate();
    return 0;
}