#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

// Rotation angles
float rotA, rotB, rotC;

// Rotated coordinates
float rx, ry, rz;

// Screen resolution
int screenW = 160;
int screenH = 44;

// Depth buffer (for hidden surface removal)
float depthBuf[160 * 44];

// ASCII character buffer (screen content)
char asciiBuf[160 * 44];

// Background character
int bgChar = ' ';

// Half-length of the cube (i.e., cube size = 2 * halfCube)
int halfCube = 10;

// Step size for sampling each face
float step = 0.6f;

// Distance from the camera to the cube
int camDistance = 60;

// 1/z for perspective projection
float invZ;

// Projected screen coordinates
int xProj, yProj;

// Projection scale factor
float scaleFactor = 40.0f;

// Rotation matrix applied to point (i, j, k)
float rotatedX(int i, int j, int k) {
    return  j * sin(rotA) * sin(rotB) * cos(rotC) - k * cos(rotA) * sin(rotB) * cos(rotC) +
            j * cos(rotA) * sin(rotC) + k * sin(rotA) * sin(rotC) + i * cos(rotB) * cos(rotC);
}

float rotatedY(int i, int j, int k) {
    return  j * cos(rotA) * cos(rotC) + k * sin(rotA) * cos(rotC) -
            j * sin(rotA) * sin(rotB) * sin(rotC) + k * cos(rotA) * sin(rotB) * sin(rotC) -
            i * cos(rotB) * sin(rotC);
}

float rotatedZ(int i, int j, int k) {
    return  k * cos(rotA) * cos(rotB) - j * sin(rotA) * cos(rotB) + i * sin(rotB);
}

// Projects a 3D point onto the screen and updates buffers if visible
void plotSurfacePoint(float cubeX, float cubeY, float cubeZ, int ch) {
    rx = rotatedX(cubeX, cubeY, cubeZ);
    ry = rotatedY(cubeX, cubeY, cubeZ);
    rz = rotatedZ(cubeX, cubeY, cubeZ) + camDistance;

    invZ = 1.0f / rz;

    // Perspective projection formula
    xProj = (int)(screenW / 2 + scaleFactor * invZ * rx * 2);
    yProj = (int)(screenH / 2 + scaleFactor * invZ * ry);

    int index = xProj + yProj * screenW;

    // Draw only if within screen bounds and closer than what's already in the buffer
    if (index >= 0 && index < screenW * screenH) {
        if (invZ > depthBuf[index]) {
            depthBuf[index] = invZ;
            asciiBuf[index] = ch;
        }
    }
}

int main(void) {
    // Clear the terminal
    printf("\x1b[2J");

    while (1) {
        // Reset buffers for this frame
        memset(depthBuf, 0, screenW * screenH * sizeof(float));
        memset(asciiBuf, bgChar, screenW * screenH);

        // Sample each cube face surface
        for (float sx = -halfCube; sx < halfCube; sx += step) {
            for (float sy = -halfCube; sy < halfCube; sy += step) {
                // Front face (z = -halfCube)
                plotSurfacePoint(sx, sy, -halfCube, '.');

                // Back face (z = +halfCube)
                plotSurfacePoint(sx, sy,  halfCube, '$');

                // Bottom face (y = -halfCube)
                plotSurfacePoint(sx, -halfCube, sy, '+');

                // Top face (y = +halfCube)
                plotSurfacePoint(sx,  halfCube, sy, '#');

                // Right face (x = +halfCube)
                plotSurfacePoint( halfCube, sx, sy, '$');

                // Left face (x = -halfCube)
                plotSurfacePoint(-halfCube, sx, sy, '-');
            }
        }

        // Move cursor to top-left
        printf("\x1b[H");

        // Print the final buffer to the screen
        for (int k = 0; k < screenW * screenH; k++) {
            putchar(k % screenW ? asciiBuf[k] : '\n');
        }

        // Increment rotation angles slowly
        rotA += 0.005f;
        rotB += 0.005f;

        // Delay for smooth animation (~60fps)
        usleep(1000);
    }

    return 0;
}