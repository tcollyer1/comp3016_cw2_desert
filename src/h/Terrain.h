#ifndef TERRAIN_H

#define TERRAIN_H

#include "..\h\Buffers.h"
#include "..\h\Texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>

#define TERRAIN_START		vec3(0.0f, -2.0f, -1.5f)

#define RENDER_DIST			512							// Map width/height
#define MAP_SIZE			(RENDER_DIST * RENDER_DIST) // Total map size

#define VERTICE_OFFSET		0.1f						// Distance between each vertice drawn on the terrain

// Global terrain positions
#define START_POS			VERTICE_OFFSET
#define END_POS				(RENDER_DIST * VERTICE_OFFSET)
#define MIDDLE_POS			(END_POS * 0.5)				// Middle coordinate of the map (x & z axes)

// Terrain texture coordinate positions for each corner of the terrain
#define BTM_LEFT			0
#define BTM_RIGHT			(RENDER_DIST - 1)
#define TOP_LEFT			(MAP_SIZE - RENDER_DIST)
#define TOP_RIGHT			(MAP_SIZE - 1)

#define ROW_CHUNKS			(RENDER_DIST - 1) // No. chunks across a single dimension
#define CHUNK_TRIANGLES		2 // Two triangles per square chunk
#define TOTAL_TRIANGLES		(ROW_CHUNKS * ROW_CHUNKS * CHUNK_TRIANGLES) // Total amount of triangles on the map

using namespace std;

// Class for creating the main terrain object.
class Terrain
{
private:
	enum Biome { GRASS, GRASS_DESERT, DESERT, DESERT_PATH, DESERT_OASIS, OASIS };

	const string assetsFolder = "media/";

	//const vec3 terrainStart = vec3(0.0f, -2.0f, -1.5f);

	// Stores all vertices - triangles across the whole map, with 11 values for each triangle
	// - 3 for vertices, 3 for colours, 3 for normals, 2 for textures
	VAO::VertexData terrainVertices[MAP_SIZE];
	int				normalsCalc[MAP_SIZE];

	VAO*			terrainVAO;

	//TerrainTexture*	textures[NUM_TEXTURES];
	vector<TerrainTexture*> textures;

	bool			trees[MAP_SIZE];

	vector<vec3>	grassModelPositions;
	vector<vec3>	oasisModelPositions;

	ivec3 terrainIndices[TOTAL_TRIANGLES];

	// For drawing
	const float drawStartPos = START_POS;
	float colVerticesOffset;
	float rowVerticesOffset;

	int colIndicesOffset;
	int rowIndicesOffset;

	// Current chunk in current row being drawn
	int rowIndex;

	void generateVertices();
	void generateLandscape();
	void setTextureCoords();
	void generateNormals();
	void createTerrainVAO();
	void setTextures(Shader* shaders);

	Biome getBiome(float terrain, float path);
	bool getIfModelPlacement(Biome biome, float noise);

public:
	Terrain(Shader* shaders);

	void getGrassModelPositions(vector<vec3>* positions);
	void getOasisModelPositions(vector<vec3>* positions);
	void offsetUserPos(vec3* pos);
	bool isAtEdge(vec3 pos);

	void drawTerrain();
};

#endif