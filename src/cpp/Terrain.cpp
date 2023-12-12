


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "..\h\Terrain.h"

// Perlin Noise
#include "..\h\FastNoiseLite.h"

#include <math.h>

using namespace glm;

#define MODEL_BOUND 0.99f

Terrain::Terrain()
{
	rowIndex			= 0;
	colVerticesOffset	= drawStartPos;
	rowVerticesOffset	= drawStartPos;
	colIndicesOffset	= 0;
	rowIndicesOffset	= 0;

	for (int i = 0; i < MAP_SIZE; i++)
	{
		trees[i] = false;
	}

	generateVertices();
	generateLandscape();
	setTextureCoords();
	generateNormals();
}

// Returns all of the vertex data for the terrain.
VAO::VertexData* Terrain::getVertices()
{
	return (terrainVertices);
}

// Gets the biome type at a given point on the terrain given the relevant
// noise values
Terrain::Biome Terrain::getBiome(float terrain, float path)
{
	Biome biome = DESERT;

	// Grassy biome. This is where plants and cacti will also appear.
	if (terrain >= 0.55f)
	{
		biome = GRASS;
	}
	// If the terrain is just below grass biome level - allow the textures
	// to mix between grass/sand
	else if (terrain < 0.55 && terrain >= 0.5)
	{
		biome = GRASS_DESERT;
	}
	// If terrain height is below -0.35, set to oasis
	// (water) biome
	else if (terrain <= -0.35f)
	{
		biome = OASIS;
	}
	// If the terrain is just above water biome level - allow the textures
	// to mix between water/sand
	// Trees will also appear here to flesh out this biome
	else if (terrain > -0.35 && terrain <= -0.3)
	{
		biome = DESERT_OASIS;
	}
	// Generate pathways - alternate sand colour to add interest to main
	// desert biome
	else if (path < 0.2f)
	{
		biome = DESERT_PATH;
	}
	// Main desert (sand) biome.
	else
	{
		biome = DESERT;
	}

	return (biome);
}

// Gets if a model should be placed at a given position, provided
// the biome and generated noise value
bool Terrain::getIfModelPlacement(Biome biome, float noise)
{
	bool model = false;

	switch (biome)
	{
	case GRASS:
	case OASIS:
		if (noise > MODEL_BOUND)
		{
			model = true;
		}
		break;
	default:
		break;
	}

	return (model);
}

void Terrain::getTreePositions(vector<vec3>* positions)
{
	for (int i = 0; i < treePositions.size(); i++)
	{
		positions->push_back(treePositions[i]);
	}
}

ivec3* Terrain::getIndices()
{
	return (terrainIndices);
}

// Generate all of the vertices and indices for the terrain
void Terrain::generateVertices()
{
	for (int i = 0; i < MAP_SIZE; i++)
	{
		// Generate vertices and colours for each triangle

		// Vertices
		terrainVertices[i].vertices.x = colVerticesOffset;	// x axis: -->
		terrainVertices[i].vertices.y = 0.0f;				// y axis - up ^ direction, so 0 - using Perlin noise for height maps
		terrainVertices[i].vertices.z = rowVerticesOffset;	// z axis - forward direction (looking at terrain at level view)

		// Set normals to 0 for now
		terrainVertices[i].normals.x = 0.0f;
		terrainVertices[i].normals.y = 0.0f;
		terrainVertices[i].normals.z = 0.0f;
		normalsCalc[i] = 0;

		// Move x position for next triangle on the grid - draws L->R >>>
		colVerticesOffset += VERTICE_OFFSET;

		// Increment chunk index within this row
		rowIndex++;

		// If at the end of the row
		if (rowIndex == RENDER_DIST)
		{
			// Reset row index
			rowIndex = 0;

			// Reset x & z positions for the next row

			// Column is reset back to the start
			colVerticesOffset = drawStartPos;
			// Row is decreased (moved backwards on z axis) - move back a row - draws front -> back ^^^
			rowVerticesOffset -= VERTICE_OFFSET;
		}
	}

	rowIndex = 0;

	// Go up in twos - 2 triangles per one chunk
	for (int i = 0; i < TOTAL_TRIANGLES; i += 2)
	{
		// Generate the indices by mapping the above to chunks (1x1 squares)

		terrainIndices[i].x = colIndicesOffset + rowIndicesOffset;					// Top left				 _
		terrainIndices[i].z = RENDER_DIST + colIndicesOffset + rowIndicesOffset;	// Bottom left			|/
		terrainIndices[i].y = 1 + colIndicesOffset + rowIndicesOffset;				// Top right

		terrainIndices[i + 1].x = 1 + colIndicesOffset + rowIndicesOffset;					// Top right		/|
		terrainIndices[i + 1].z = RENDER_DIST + colIndicesOffset + rowIndicesOffset;		// Bottom left		-
		terrainIndices[i + 1].y = 1 + RENDER_DIST + colIndicesOffset + rowIndicesOffset;	// Bottom right

		// Move x position for next triangle on the grid
		colIndicesOffset += 1;

		// Increment chunk index within this row
		rowIndex++;

		// If at the end of the row
		if (rowIndex == ROW_CHUNKS)
		{
			// Reset row index
			rowIndex = 0;

			// Reset x & z positions for the next row

			// Column is reset back to the start
			colIndicesOffset = 0;
			// Move back a row
			rowIndicesOffset = rowIndicesOffset + RENDER_DIST;
		}
	}
}

// Generate height maps for the terrain with Perlin noise,
// as well as create colour map based on biomes as a template for
// the textures for the shader.
void Terrain::generateLandscape()
{
	// Assign perlin noise type for the map. This affects the y axis
	FastNoiseLite terrainNoise;
	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	// Set noise scale
	terrainNoise.SetFrequency(0.025f);
	// Generate random seed
	int seed = rand() % 100;
	terrainNoise.SetSeed(seed);

	// Perlin noise for pathway map
	FastNoiseLite pathNoise;
	pathNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	pathNoise.SetFrequency(0.05f);
	int pSeed = rand() % 100;
	pathNoise.SetSeed(pSeed);

	// Perlin noise for tree placement
	FastNoiseLite trees;
	trees.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	trees.SetFrequency(10.0f);
	int tSeed = rand() % 100;
	trees.SetSeed(tSeed);

	// Terrain vertice index
	int terrainIndex = 0;

	float terrainVal = 0.0f;
	float pathVal = 0.0f;
	float treesVal = 0.0f;

	Biome currBiome = DESERT;

	for (int x = 0; x < RENDER_DIST; x++)
	{
		for (int y = 0; y < RENDER_DIST; y++)
		{
			// Get noise values for biome type and terrain height (between -1 and 1)
			// at the given x/y coordinate (2D position)

			// Generate noise at 3 different frequencies for additional variation
			terrainVal = 1 * terrainNoise.GetNoise((float)x, (float)y)
				+ 0.5 * terrainNoise.GetNoise(2 * (float)x, 2 * (float)y)
				+ 0.25 * terrainNoise.GetNoise(4 * (float)x, 4 * (float)y);

			// Generate noise and get absolute value (turbulence).
			// This produces a simple noise map that gives the impression of
			// winding pathways
			pathVal = abs(pathNoise.GetNoise((float)x, (float)y));

			treesVal = trees.GetNoise((float)x, (float)y);

			// Set random height value (random noise) calculated before,
			// to the vertex y value. 
			// Divide by the sum of the 3 amplitudes to maintain values between 0-1 (?)
			// Multiply by 2 for greater height diversity.
			terrainVertices[terrainIndex].vertices.y = (terrainVal / (1 + 0.5 + 0.25)) * 2;

			currBiome = getBiome(terrainVal, pathVal);

			// Grass
			if (currBiome == GRASS)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;

				// Set grass/cacti here
				if (getIfModelPlacement(currBiome, treesVal))
				{
					treePositions.push_back(vec3(terrainVertices[terrainIndex].vertices));
				}
			}
			// Grass-desert transition
			else if (currBiome == GRASS_DESERT)
			{
				terrainVertices[terrainIndex].colours.r = 0.5f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}
			// Water
			else if (currBiome == OASIS)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 1.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}
			// Desert-water transition
			else if (currBiome == DESERT_OASIS)
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.5f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}
			// Sandy pathway
			else if (currBiome == DESERT_PATH)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 1.0f;
			}
			// Normal sand
			else
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
				terrainVertices[terrainIndex].colours.a = 0.0f;
			}

			terrainIndex++;
		}
	}
}

// Calculate the texture coordinates for the terrain object.
void Terrain::setTextureCoords()
{
	// Texture coords - bottom left
	terrainVertices[BTM_LEFT].textures.x = 0.0f;
	terrainVertices[BTM_LEFT].textures.y = 0.0f;

	// Bottom right
	terrainVertices[BTM_RIGHT].textures.x = 1.0f;
	terrainVertices[BTM_RIGHT].textures.y = 0.0f;

	// Top left
	terrainVertices[TOP_LEFT].textures.x = 0.0f;
	terrainVertices[TOP_LEFT].textures.y = 1.0f;

	// Top right
	terrainVertices[TOP_RIGHT].textures.x = 1.0f;
	terrainVertices[TOP_RIGHT].textures.y = 1.0f;

	int z = 0;

	for (int x = 0; x < MAP_SIZE; x++)
	{
		div_t divResultX;
		div_t divResultZ;

		divResultX = div(x, RENDER_DIST);
		divResultZ = div(z, RENDER_DIST);

		if (x != BTM_LEFT
			&& x != BTM_RIGHT
			&& x != TOP_LEFT
			&& x != TOP_RIGHT)
		{
			terrainVertices[x].textures.x = (float)divResultX.rem / RENDER_DIST;
			terrainVertices[x].textures.y = (float)divResultZ.rem / RENDER_DIST;
		}

		if (divResultX.rem == RENDER_DIST - 1)
		{
			z++;
		}
	}
}

// Calculate the normal map for the terrain for BlinnPhong lighting.
void Terrain::generateNormals()
{
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		div_t divResult;
		divResult = div(i, RENDER_DIST);

		// If not at the right edge
		if (divResult.rem != RENDER_DIST - 1)
		{
			vec3 topLeft = vec3(terrainVertices[i].vertices);
			vec3 topRight = vec3(terrainVertices[i + 1].vertices);

			vec3 btmLeft = vec3(terrainVertices[i - RENDER_DIST].vertices);

			// Multiply by -1 to ensure lighting doesn't appear reversed
			vec3 normal = normalize(cross((topRight - topLeft), (btmLeft - topLeft)) * -1.0f);

			// Top left of quad
			terrainVertices[i].normals.x += normal.x;
			terrainVertices[i].normals.y += normal.y;
			terrainVertices[i].normals.z += normal.z;
			normalsCalc[i]++;

			// Top right of quad
			terrainVertices[i + 1].normals.x += normal.x;
			terrainVertices[i + 1].normals.y += normal.y;
			terrainVertices[i + 1].normals.z += normal.z;
			normalsCalc[i + 1]++;

			// Bottom left of quad
			terrainVertices[i - RENDER_DIST].normals.x += normal.x;
			terrainVertices[i - RENDER_DIST].normals.y += normal.y;
			terrainVertices[i - RENDER_DIST].normals.z += normal.z;
			normalsCalc[i - RENDER_DIST]++;

			// Bottom right of quad
			terrainVertices[(i - RENDER_DIST) + 1].normals.x += normal.x;
			terrainVertices[(i - RENDER_DIST) + 1].normals.y += normal.y;
			terrainVertices[(i - RENDER_DIST) + 1].normals.z += normal.z;
			normalsCalc[(i - RENDER_DIST) + 1]++;
		}
	}

	// Average normals where a vertice has multiple normal values
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		terrainVertices[i].normals.x /= (float)normalsCalc[i];
		terrainVertices[i].normals.y /= (float)normalsCalc[i];
		terrainVertices[i].normals.z /= (float)normalsCalc[i];
	}
}