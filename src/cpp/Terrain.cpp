


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "..\h\Terrain.h"

// Perlin Noise
#include "..\h\FastNoiseLite.h"

#include <math.h>

using namespace glm;

Terrain::Terrain()
{
	rowIndex			= 0;
	colVerticesOffset	= drawStartPos;
	rowVerticesOffset	= drawStartPos;
	colIndicesOffset	= 0;
	rowIndicesOffset	= 0;

	generateVertices();
	generateLandscape();
	setTextureCoords();
	generateNormals();
}

VAO::VertexData* Terrain::getVertices()
{
	return (terrainVertices);
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

		// So top right should match top right,
		// Bottom left should match bottom left,
		// Bottom right (on bottom triangle) should be the opposite of top left (top triangle)

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
	// Assign perlin noise type for the map. This affects the y axis, which we previously
	// assigned a value of 0 for flat terrain
	FastNoiseLite terrainNoise;
	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	// Set noise scale
	//terrainNoise.SetFrequency(0.05f);
	terrainNoise.SetFrequency(0.025f);

	// Generate random seed
	int seed = rand() % 100;

	terrainNoise.SetSeed(seed);

	// Create another biome with another FastNoiseLite var
	FastNoiseLite biomeNoise;

	// This time, set the noise type to cellular
	biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);

	biomeNoise.SetFrequency(0.05f);

	// Generate biome random seed
	int bSeed = rand() % 100;

	terrainNoise.SetSeed(seed);
	biomeNoise.SetSeed(bSeed);

	// Terrain vertice index
	int terrainIndex = 0;
	float biomeVal = 0.0f;
	float terrainVal = 0.0f;

	for (int x = 0; x < RENDER_DIST; x++)
	{
		for (int y = 0; y < RENDER_DIST; y++)
		{
			// Get noise values for biome type and terrain height (between -1 and 1)
			// at the given x/y coordinate (2D position)
			biomeVal = biomeNoise.GetNoise((float)x, (float)y);
			//terrainVal = terrainNoise.GetNoise((float)x, (float)y);

			// Generate noise at 3 different frequencies for additional variation
			terrainVal = 1 * terrainNoise.GetNoise((float)x, (float)y)
				+ 0.5 * terrainNoise.GetNoise(2 * (float)x, 2 * (float)y)
				+ 0.25 * terrainNoise.GetNoise(4 * (float)x, 4 * (float)y);

			// If the terrain height exceeds 0.35, green for grassy
			// biome
			if (terrainVal >= 0.35f)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
			}
			// If the terrain is just below grass biome level - allow the textures
			// to mix between grass/sand
			else if (terrainVal < 0.35 && terrainVal >= 0.3)
			{
				terrainVertices[terrainIndex].colours.r = 0.5f;
				terrainVertices[terrainIndex].colours.g = 1.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
			}
			// If terrain height is below -0.35, set to oasis
			// (water) biome
			else if (terrainVal <= -0.35f)
			{
				terrainVertices[terrainIndex].colours.r = 0.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 1.0f;
			}
			// If the terrain is just above water biome level - allow the textures
			// to mix between water/sand
			else if (terrainVal > -0.35 && terrainVal <= -0.3)
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.5f;
			}

			// Otherwise, if the biome value generated is -0.9 or below, 
			// let's say this is a "plains" biome (green)
			//else if (biomeVal <= -0.8f)
			//{
			//	// Set colour for plains
			//	/*terrainVertices[terrainIndex][3] = 0.0f;
			//	terrainVertices[terrainIndex][4] = 0.75f;
			//	terrainVertices[terrainIndex][5] = 0.25f;*/

			//	terrainVertices[terrainIndex][3] = 0.0f;
			//	terrainVertices[terrainIndex][4] = 1.0f;
			//	terrainVertices[terrainIndex][5] = 0.0f;
			//}
			// Main desert (sand) biome
			else
			{
				terrainVertices[terrainIndex].colours.r = 1.0f;
				terrainVertices[terrainIndex].colours.g = 0.0f;
				terrainVertices[terrainIndex].colours.b = 0.0f;
			}

			// Set random height value (random noise) calculated before,
			// to the vertex y value. 
			// Divide by the sum of the 3 amplitudes to maintain values between 0-1 (?)
			// Multiply by 2 for greater height diversity.
			terrainVertices[terrainIndex].vertices.y = (terrainVal / (1 + 0.5 + 0.25)) * 2;

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
			// Texture coordinates
			/*terrainVertices[i][9] = 0.0f;
			terrainVertices[i][10] = 1.0f;*/
			normalsCalc[i]++;

			// Top right of quad
			terrainVertices[i + 1].normals.x += normal.x;
			terrainVertices[i + 1].normals.y += normal.y;
			terrainVertices[i + 1].normals.z += normal.z;
			// Texture coordinates
			/*terrainVertices[i+1][9] = 1.0f;
			terrainVertices[i+1][10] = 1.0f;*/
			normalsCalc[i + 1]++;

			// Bottom left of quad
			terrainVertices[i - RENDER_DIST].normals.x += normal.x;
			terrainVertices[i - RENDER_DIST].normals.y += normal.y;
			terrainVertices[i - RENDER_DIST].normals.z += normal.z;
			// Texture coordinates
			/*terrainVertices[i - RENDER_DIST][9] = 0.0f;
			terrainVertices[i - RENDER_DIST][10] = 0.0f;*/
			normalsCalc[i - RENDER_DIST]++;

			// Bottom right of quad
			terrainVertices[(i - RENDER_DIST) + 1].normals.x += normal.x;
			terrainVertices[(i - RENDER_DIST) + 1].normals.y += normal.y;
			terrainVertices[(i - RENDER_DIST) + 1].normals.z += normal.z;
			// Texture coordinates
			/*terrainVertices[(i - RENDER_DIST) + 1][9] = 1.0f;
			terrainVertices[(i - RENDER_DIST) + 1][10] = 0.0f;*/
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