#include <stdio.h>
#include <raylib.h>

#include "rlgl.h"

#define WIDTH 900
#define HEIGHT 600

void ApplicaAtlasA_MeshCubo(Mesh *mesh, int atlasColonne, int atlasRighe,
                             int tileX[6], int tileY[6], int tileRot[6])
{
    float tileWidth  = 1.0f / atlasColonne;
    float tileHeight = 1.0f / atlasRighe;
 
    for (int i = 0; i < 6; i++) {
        float uMin = tileX[i] * tileWidth;
        float uMax = (tileX[i] + 1) * tileWidth;
        float vMin = tileY[i] * tileHeight;
        float vMax = (tileY[i] + 1) * tileHeight;
 
        float corners[4][2] = {
            { uMin, vMin },
            { uMin, vMax },
            { uMax, vMax },
            { uMax, vMin },
        };
 
        int indicePartenza = i * 8;
        int rot = ((tileRot[i] % 4) + 4) % 4;
 
        for (int c = 0; c < 4; c++) {
            int sorgente = (c + rot) % 4;
            mesh->texcoords[indicePartenza + c * 2 + 0] = corners[sorgente][0];
            mesh->texcoords[indicePartenza + c * 2 + 1] = corners[sorgente][1];
        }
    }
}

int main(int argc, char *argv[])
{
	InitWindow(WIDTH, HEIGHT, "3D Cube");

	SetTargetFPS(60);

	Vector3 pos = {5, 0, 0};
	Vector3 target = {0, 0, 0};
	Vector3 up = {0, 0, 1};
	float fovy = 90.0;
	Camera3D cam = {pos, target, up, fovy, CAMERA_PERSPECTIVE};

	Texture2D textureAtlas = LoadTexture("texture-atlas-minecraft.png");

	Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);

	int colonneAtlas = 16;
	int righeAtlas = 16;
	
	int textureX[6] = { 0, 2, 3, 3, 3, 3 };
	int textureY[6] = { 0, 0, 0, 0, 0, 0 };
	int textureRot[6] = { 0, 2, 2, 1, 1, 2 };

	ApplicaAtlasA_MeshCubo(&cubeMesh, colonneAtlas, righeAtlas, textureX, textureY, textureRot);
	
	UpdateMeshBuffer(cubeMesh, 1, cubeMesh.texcoords, cubeMesh.vertexCount * 2 * sizeof(float), 0);
	
	Model cuboModel = LoadModelFromMesh(cubeMesh);
	
	cuboModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = textureAtlas;	

	int chunk = 16;
	
	while (!WindowShouldClose())
	{
		UpdateCamera(&cam, CAMERA_FREE);
		
		BeginDrawing();
		ClearBackground(SKYBLUE);
		BeginMode3D(cam);
	
		Vector3 origin = {0, 0, 0};
		
		for (float i = 0; i < chunk; i++){
			for (float j = 0; j < chunk; j++){
				for (float k = 0; k < chunk; k++){				
					DrawModel(cuboModel, (Vector3){ i*2, j*2, k*2 }, 1.0f, WHITE);	
				}
			}	
		}
		
		EndMode3D();
		EndDrawing();
	}
	return 0;
}

