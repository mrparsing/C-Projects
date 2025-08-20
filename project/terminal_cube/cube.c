#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

float rotA, rotB, rotC;

float rx, ry, rz;

int screenW = 160;
int screenH = 44;

float depthBuf[160 * 44];

char asciiBuf[160 * 44];

int bgChar = ' ';

int halfCube = 10;

float step = 0.6f;

int camDistance = 60;

float invZ;

int xProj, yProj;

float scaleFactor = 40.0f;

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

void plotSurfacePoint(float cubeX, float cubeY, float cubeZ, int ch) {
    rx = rotatedX(cubeX, cubeY, cubeZ);
    ry = rotatedY(cubeX, cubeY, cubeZ);
    rz = rotatedZ(cubeX, cubeY, cubeZ) + camDistance;

    invZ = 1.0f / rz;

    xProj = (int)(screenW / 2 + scaleFactor * invZ * rx * 2);
    yProj = (int)(screenH / 2 + scaleFactor * invZ * ry);

    int index = xProj + yProj * screenW;

    if (index >= 0 && index < screenW * screenH) {
        if (invZ > depthBuf[index]) {
            depthBuf[index] = invZ;
            asciiBuf[index] = ch;
        }
    }
}

int main(void) {
    printf("\x1b[2J");

    while (1) {
        memset(depthBuf, 0, screenW * screenH * sizeof(float));
        memset(asciiBuf, bgChar, screenW * screenH);

        for (float sx = -halfCube; sx < halfCube; sx += step) {
            for (float sy = -halfCube; sy < halfCube; sy += step) {
                plotSurfacePoint(sx, sy, -halfCube, '.');
                plotSurfacePoint(sx, sy,  halfCube, '$');
                plotSurfacePoint(sx, -halfCube, sy, '+');
                plotSurfacePoint(sx,  halfCube, sy, '#');
                plotSurfacePoint( halfCube, sx, sy, '$');
                plotSurfacePoint(-halfCube, sx, sy, '-');
            }
        }

        printf("\x1b[H");

        for (int k = 0; k < screenW * screenH; k++) {
            putchar(k % screenW ? asciiBuf[k] : '\n');
        }

        rotA += 0.005f;
        rotB += 0.005f;

        usleep(1000);
    }

    return 0;
}