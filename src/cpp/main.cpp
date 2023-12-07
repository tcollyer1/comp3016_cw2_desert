// See labs 5-6

#include <iostream>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Textures - header must be in C:/Users/Public/OpenGL/include
#include "stb_image.h"

// Shaders
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
//#include "shaders/LoadShaders.h"

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Perlin Noise
#include "..\h\FastNoiseLite.h"

// Other
#include "..\h\main.h"
#include <math.h>

using namespace std;
using namespace glm;

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define RENDER_DIST			512 // Map width/height
#define MAP_SIZE			(RENDER_DIST * RENDER_DIST) // Total map size

#define VERTICE_OFFSET		0.1f // Distance between each vertice drawn on the terrain

#define LIGHT_ORBIT_OFFSET	8.0f;

#define START_POS			0.1f
#define END_POS				(RENDER_DIST * VERTICE_OFFSET)
#define MIDDLE_POS			(END_POS * 0.5) // Middle coordinate of the map (x & z axes)

// Terrain texture coordinate positions for each corner of the terrain
#define BTM_LEFT		0
#define BTM_RIGHT		(RENDER_DIST - 1)
#define TOP_LEFT		(MAP_SIZE - RENDER_DIST)
#define TOP_RIGHT		(MAP_SIZE - 1)

GLuint program;
GLuint lightProg;

/*******************************************************************************************
*
* PROC. TERRAIN GENERATION - GLOBALS
*
*******************************************************************************************/
// No. chunks across a single dimension
const int rowChunks = RENDER_DIST - 1;
// Two triangles per square chunk
const int trianglesPerChunk = 2;
// Total amount of triangles on the map
const int totalTriangles = rowChunks * rowChunks * trianglesPerChunk;

// Stores all vertices - triangles across the whole map, with 11 values for each triangle
// - 3 for vertices, 3 for colours, 3 for normals, 2 for textures
GLfloat terrainVertices[MAP_SIZE][11];

int normalsCalc[MAP_SIZE];

// Stores all indices - maps the above vertices to chunks by connecting the triangles into
// quads
GLuint terrainIndices[totalTriangles][3];

// For drawing
float drawStartPos		= START_POS;
float colVerticesOffset = drawStartPos;
float rowVerticesOffset = drawStartPos;

int colIndicesOffset	= 0;
int rowIndicesOffset	= 0;

// Current chunk in current row being drawn
int rowIndex = 0;

/*******************************************************************************************
*
* MATRIX GLOBALS
*
*******************************************************************************************/
// Global matrices for transformations
mat4 transform;
mat4 model;
mat4 view;
mat4 projection;

/*******************************************************************************************
*
* CAMERA POSITION GLOBALS
*
*******************************************************************************************/
// Relative pos within world space
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
// Travel direction - forward, since we've set it to negative on Z axis, which usually
// points towards you
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
// Absolute up direction
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

// Horizontal rotation
float cameraYaw = -90.0f;
// Vertical rotation
float cameraPitch = 0.0f;

// If it's the first time the mouse is entering the window, this determines whether to set
// default last x/y positions or not (below). If it's first entry, it has no previous 
// positions so must set them
bool mouseFirstEntry = true;

// Camera positions from the last frame
float cameraLastXPos = 0.0f;
float cameraLastYPos = 0.0f;

/*******************************************************************************************
*
* TIME GLOBALS
*
*******************************************************************************************/
// Time diff - time taken between each frame change
float deltaTime = 0.0f;
// Last value of time diff - last current frame
float lastFrame = 0.0f;

/*******************************************************************************************
*
* VAOS & BUFFERS FOR VERTEX DATA
*
*******************************************************************************************/
// VAO vertex attribute types
enum VAO_ID { TRIANGLE, INDICES, COLOUR, TEXTURE1, TEXTURE2, TEXTURE3, NORMALS, NUM_VAO = 2 };
GLuint VAOs[NUM_VAO];

// Buffer types
enum BUFFER_ID { ARRAY_BUFFER, NUM_BUFFER = 10 };
GLuint buffers[NUM_BUFFER];

/*******************************************************************************************
*
* LIGHTING
*
*******************************************************************************************/
// Set default light position
//vec3 lightPos(1.2f, 5.0f, 2.0f);
vec3 lightPos = vec3(MIDDLE_POS, MIDDLE_POS, -MIDDLE_POS);
GLuint lightVAO;
GLuint lightVBO;

vec3 day1 = vec3(0.0f, 0.6f, 1.0f); // Midday
vec3 day2 = vec3(1.0f, 0.5f, 0.0f); // Sunset
vec3 day3 = vec3(0.0f, 0.0f, 0.1f); // Midnight
vec3 day4 = vec3(0.9f, 0.0f, 0.2f); // Sunrise

vec3 currSkyColour = day1;

vec3 terrainStart = vec3(0.0f, -2.0f, -1.5f);

/*
	Main Steps of OpenGL Object Rendering
	-------------------------------------
*/
/* The main steps are as follows:
*		1. Instantiating spatial information (window, size/height) in C++ --> CPU
*		2. Transitioning data to the shader using OpenGL --> CPU->GPU
*		3. Rendering with GLSL (OpenGL shading language) --> GPU
*/

int main()
{
	// Initialise GLFW - used for user I/O, creating windows and more using OpenGL
	glfwInit();

	// Create a GLFWwindow window
	//GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Desert", NULL, NULL);
	GLFWwindow* window = glfwCreateWindow(glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
							glfwGetVideoMode(glfwGetPrimaryMonitor())->height, "Desert",
							glfwGetPrimaryMonitor(), NULL);

	// Checks for successful window creation
	if (window == NULL)
	{
		cout << "GLFW window could not instantiate\n";
		glfwTerminate();

		return -1;
	}

	// Set cursor to bind to window, hides cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Assign window to current OpenGL context
	glfwMakeContextCurrent(window);

	// GLEW is used to render a window instantiated using GLFW.
	// Initialise GLEW - checks what OpenGL extensions are/are not supported
	//glewInit();
	// Init GLAD instead
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialise GLAD\n";

		return -1;
	}

	// Set shaders & model
	Shader terrainShaders("shaders/vertexShader.vert", "shaders/fragmentShader.frag");
	Shader lightShaders("shaders/lightShader.vert", "shaders/lightShader.frag");
	Shader modelShaders("shaders/modelShader.vert", "shaders/modelShader.frag");

	Model rock("media/rock/Rock07-Base.obj");

	/*ShaderInfo shaders[] =
	{
		{ GL_VERTEX_SHADER, "shaders/vertexShader.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/fragmentShader.frag"},
		{ GL_NONE, NULL }
	};

	ShaderInfo lightShaders[] =
	{
		{ GL_VERTEX_SHADER, "shaders/lightShader.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/lightShader.frag"},
		{ GL_NONE, NULL }
	};

	program = LoadShaders(shaders);
	lightProg = LoadShaders(lightShaders);*/

	// Width and height should match that of the window
	// 0, 0 indicates the position of the window
	glViewport(0, 0, glfwGetVideoMode(glfwGetPrimaryMonitor())->width, glfwGetVideoMode(glfwGetPrimaryMonitor())->height);

	// Enables "depth" in textures - stops lack of depth in 3D objects when
	// texturing & strange warping
	glEnable(GL_DEPTH_TEST);

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND); // Enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Assigns frameBufferSizeCallback() as the callback function to dynamically handle 
	// window resizing.
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

	// Set mouse callback for mouse movement
	glfwSetCursorPosCallback(window, mouseCallback);


	/*******************************************************************************************
	* 
	* --- MAIN CODE ---
	* 
	*******************************************************************************************/

	// Cube vertices - for light
	float verticesCube[] =
	{
		// Position				// Normals				// Texture
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f, // Face 1, triangle 1
		 0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 1.0f, // Face 1, triangle 2
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 0.0f, // Face 2
		 0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f, 1.0f,		0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f, // Face 3... etc.
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f,  0.0f,		0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f,  1.0f,  0.0f,		0.0f, 1.0f
	};


	/*******************************************************************************************
	*
	* SETTING UP VERTICES & INDICES
	*
	*******************************************************************************************/

	for (int i = 0; i < MAP_SIZE; i++)
	{
		// Generate vertices and colours for each triangle

		// Vertices
		terrainVertices[i][0] = colVerticesOffset;	// x axis: -->
		terrainVertices[i][1] = 0.0f;				// y axis - up ^ direction, so 0 - using Perlin noise for height maps
		terrainVertices[i][2] = rowVerticesOffset;	// z axis - forward direction (looking at terrain at level view)

		// Colour
		/*terrainVertices[i][3] = 0.0f;
		terrainVertices[i][4] = 0.75f;
		terrainVertices[i][5] = 0.25f;*/

		// Set normals to 0 for now
		terrainVertices[i][6] = 0.0f;
		terrainVertices[i][7] = 0.0f;
		terrainVertices[i][8] = 0.0f;
		normalsCalc[i]		  = 0;

		// Move x position for next triangle on the grid - draws L->R >>>
		//colVerticesOffset -= 0.0625f;
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
	for (int i = 0; i < totalTriangles; i+=2)
	{
		// Generate the indices by mapping the above to chunks (1x1 squares)

		// So top right should match top right,
		// Bottom left should match bottom left,
		// Bottom right (on bottom triangle) should be the opposite of top left (top triangle)

		terrainIndices[i][0] = colIndicesOffset + rowIndicesOffset;					// Top left              _
		terrainIndices[i][2] = RENDER_DIST + colIndicesOffset + rowIndicesOffset;	// Bottom left          |/
		terrainIndices[i][1] = 1 + colIndicesOffset + rowIndicesOffset;				// Top right

		terrainIndices[i+1][0] = 1 + colIndicesOffset + rowIndicesOffset;				// Top right		 /|
		terrainIndices[i+1][2] = RENDER_DIST + colIndicesOffset + rowIndicesOffset;		// Bottom left		 -
		terrainIndices[i+1][1] = 1 + RENDER_DIST + colIndicesOffset + rowIndicesOffset; // Bottom right

		// Move x position for next triangle on the grid
		colIndicesOffset += 1;

		// Increment chunk index within this row
		rowIndex++;

		// If at the end of the row
		if (rowIndex == rowChunks)
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

	/*******************************************************************************************
	*
	* PERLIN NOISE
	*
	*******************************************************************************************/
	 // Assign perlin noise type for the map. This affects the y axis, which we previously
	 // assigned a value of 0 for flat terrain
	FastNoiseLite terrainNoise;
	terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

	// Set noise scale
	//terrainNoise.SetFrequency(0.05f);
	terrainNoise.SetFrequency(0.025f);

	// Generate random seed
	//int seed = rand() % 100;

	//terrainNoise.SetSeed(seed);

	// Create another biome with another FastNoiseLite var
	FastNoiseLite biomeNoise;

	// This time, set the noise type to cellular
	biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);

	biomeNoise.SetFrequency(0.05f);

	// Generate biome random seed
	int bSeed = rand() % 100;

	terrainNoise.SetSeed(bSeed);

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
				terrainVertices[terrainIndex][3] = 0.0f;
				terrainVertices[terrainIndex][4] = 1.0f;
				terrainVertices[terrainIndex][5] = 0.0f;
			}
			// If the terrain is just below grass biome level - allow the textures
			// to mix between grass/sand
			else if (terrainVal < 0.35 && terrainVal >= 0.3)
			{
				terrainVertices[terrainIndex][3] = 0.5f;
				terrainVertices[terrainIndex][4] = 1.0f;
				terrainVertices[terrainIndex][5] = 0.0f;
			}
			// If terrain height is below -0.35, set to oasis
			// (water) biome
			else if (terrainVal <= -0.35f)
			{
				terrainVertices[terrainIndex][3] = 0.0f;
				terrainVertices[terrainIndex][4] = 0.0f;
				terrainVertices[terrainIndex][5] = 1.0f;
			}
			// If the terrain is just above water biome level - allow the textures
			// to mix between water/sand
			else if (terrainVal > -0.35 && terrainVal <= -0.3)
			{
				terrainVertices[terrainIndex][3] = 1.0f;
				terrainVertices[terrainIndex][4] = 0.0f;
				terrainVertices[terrainIndex][5] = 0.5f;
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
				terrainVertices[terrainIndex][3] = 1.0f;
				terrainVertices[terrainIndex][4] = 0.0f;
				terrainVertices[terrainIndex][5] = 0.0f;
			}

			// Set random height value (random noise) calculated before,
			// to the vertex y value. 
			// Divide by the sum of the 3 amplitudes to maintain values between 0-1 (?)
			// Multiply by 2 for greater height diversity.
			terrainVertices[terrainIndex][1] = (terrainVal / (1 + 0.5 + 0.25)) * 2;

			terrainIndex++;
		}
	}

	/*******************************************************************************************
	*
	* CALCULATING TEXTURE COORDS
	*
	*******************************************************************************************/
	// Texture coords - bottom left
	terrainVertices[BTM_LEFT][9] = 0.0f;
	terrainVertices[BTM_LEFT][10] = 0.0f;

	// Bottom right
	terrainVertices[BTM_RIGHT][9] = 1.0f;
	terrainVertices[BTM_RIGHT][10] = 0.0f;

	// Top left
	terrainVertices[TOP_LEFT][9] = 0.0f;
	terrainVertices[TOP_LEFT][10] = 1.0f;

	// Top right
	terrainVertices[TOP_RIGHT][9] = 1.0f;
	terrainVertices[TOP_RIGHT][10] = 1.0f;

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
			terrainVertices[x][9] = (float)divResultX.rem / RENDER_DIST;
			terrainVertices[x][10] = (float)divResultZ.rem / RENDER_DIST;
		}		

		if (divResultX.rem == RENDER_DIST - 1)
		{
			z++;
		}		
	}


	/*******************************************************************************************
	*
	* COMPUTING THE NORMALS OF THE TERRAIN
	*
	*******************************************************************************************/
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		div_t divResult;
		divResult = div(i, RENDER_DIST);

		// If not at the right edge
		if (divResult.rem != RENDER_DIST - 1)
		{
			vec3 topLeft = vec3(terrainVertices[i][0], terrainVertices[i][1], terrainVertices[i][2]);
			vec3 topRight = vec3(terrainVertices[i + 1][0], terrainVertices[i + 1][1], terrainVertices[i + 1][2]);

			vec3 btmLeft = vec3(terrainVertices[i - RENDER_DIST][0], terrainVertices[i - RENDER_DIST][1], terrainVertices[i - RENDER_DIST][2]);

			// Multiply by -1 to ensure lighting doesn't appear reversed
			vec3 normal = normalize(cross((topRight - topLeft), (btmLeft - topLeft)) * -1.0f);

			// Top left of quad
			terrainVertices[i][6] += normal.x;
			terrainVertices[i][7] += normal.y;
			terrainVertices[i][8] += normal.z;
			// Texture coordinates
			/*terrainVertices[i][9] = 0.0f;
			terrainVertices[i][10] = 1.0f;*/
			normalsCalc[i]++;

			// Top right of quad
			terrainVertices[i + 1][6] += normal.x;
			terrainVertices[i + 1][7] += normal.y;
			terrainVertices[i + 1][8] += normal.z;
			// Texture coordinates
			/*terrainVertices[i+1][9] = 1.0f;
			terrainVertices[i+1][10] = 1.0f;*/
			normalsCalc[i+1]++;

			// Bottom left of quad
			terrainVertices[i - RENDER_DIST][6] += normal.x;
			terrainVertices[i - RENDER_DIST][7] += normal.y;
			terrainVertices[i - RENDER_DIST][8] += normal.z;
			// Texture coordinates
			/*terrainVertices[i - RENDER_DIST][9] = 0.0f;
			terrainVertices[i - RENDER_DIST][10] = 0.0f;*/
			normalsCalc[i - RENDER_DIST]++;
			
			// Bottom right of quad
			terrainVertices[(i - RENDER_DIST) + 1][6] += normal.x;
			terrainVertices[(i - RENDER_DIST) + 1][7] += normal.y;
			terrainVertices[(i - RENDER_DIST) + 1][8] += normal.z;
			// Texture coordinates
			/*terrainVertices[(i - RENDER_DIST) + 1][9] = 1.0f;
			terrainVertices[(i - RENDER_DIST) + 1][10] = 0.0f;*/
			normalsCalc[(i - RENDER_DIST) + 1]++;
		}		
	}

	// Average normals where a vertice has multiple normal values
	for (int i = RENDER_DIST; i < MAP_SIZE; i++)
	{
		terrainVertices[i][6] /= (float)normalsCalc[i];
		terrainVertices[i][7] /= (float)normalsCalc[i];
		terrainVertices[i][8] /= (float)normalsCalc[i];
	}

	/*******************************************************************************************
	*
	* CREATING THE VAO FOR VERTICES AND COLOURS
	*
	*******************************************************************************************/

	// Set index of the VAO.
	glGenVertexArrays(NUM_VAO, VAOs);

	// Bind VAO to an array buffer.
	glBindVertexArray(VAOs[0]);

	// Sets indexes for all required buffer objects - this is where we're storing the vertices.
	glGenBuffers(NUM_BUFFER, buffers);

	// Bind VAO to an array buffer type for drawing.
	glBindBuffer(GL_ARRAY_BUFFER, buffers[TRIANGLE]);

	// Allocate buffer memory for the vertices we established before.
	glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

	// Bind buffer for indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDICES]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), terrainIndices, GL_STATIC_DRAW);
	
	// Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Textures
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Colours
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(3);

	/*******************************************************************************************
	*
	* TEXTURES
	*
	*******************************************************************************************/

	// Assign textures to generate and bind to 2D texture
	glGenTextures(NUM_BUFFER, buffers);

	//-------------------------
	// TEXTURE 1: SAND
	//-------------------------

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE1]);

	// Selects x axis of texture bound to GL_TEXTURE_2D & sets to repeat beyond 
	// normalised coordinates (WRAP_S)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// Selects y axis and does the same (WRAP_T)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Texture scaling/filtering. Params: buffer type | downscaling (GL_TEXTURE_MIN_FILTER) or upscaling (GL_TEXTURE_MAG_FILTER) | filtering type
	// Sets to use linear interpolation between adjacent mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Sets to use linear interpolation upscaling (past largest mipmap texture) (default upscaling method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//// Sets bound texture to use nearest neighbour downscaling (enlargens the image)
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//// Sets to also use nearest neighbour upscaling - appears more pixellated.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// Load texture and set width, height and colour channel params
	int width, height, colourChannels;
	unsigned char* data = stbi_load("media/sand.jpg", &width, &height, &colourChannels, 0);

	if (data) // If retrieval was successful
	{
		// Generation of texture from retrieved texture data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		// Automatically generates all required mipmaps on bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // If retrieval unsuccessful
	{
		cout << "Failed to load texture.\n";
		return -1;
	}

	// Clears retrieved texture from memory
	stbi_image_free(data);

	//-------------------------
	// TEXTURE 2: GRASS
	//-------------------------

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE2]);

	// Selects x axis of texture bound to GL_TEXTURE_2D & sets to repeat beyond 
	// normalised coordinates (WRAP_S)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// Selects y axis and does the same (WRAP_T)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Texture scaling/filtering. Params: buffer type | downscaling (GL_TEXTURE_MIN_FILTER) or upscaling (GL_TEXTURE_MAG_FILTER) | filtering type
	// Sets to use linear interpolation between adjacent mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Sets to use linear interpolation upscaling (past largest mipmap texture) (default upscaling method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load texture and set width, height and colour channel params
	data = stbi_load("media/grass.jpg", &width, &height, &colourChannels, 0);

	if (data) // If retrieval was successful
	{
		// Generation of texture from retrieved texture data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		// Automatically generates all required mipmaps on bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // If retrieval unsuccessful
	{
		cout << "Failed to load texture.\n";
		return -1;
	}

	// Clears retrieved texture from memory
	stbi_image_free(data);


	//-------------------------
	// TEXTURE 3: WATER
	//-------------------------

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE3]);

	// Selects x axis of texture bound to GL_TEXTURE_2D & sets to repeat beyond 
	// normalised coordinates (WRAP_S)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// Selects y axis and does the same (WRAP_T)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Texture scaling/filtering. Params: buffer type | downscaling (GL_TEXTURE_MIN_FILTER) or upscaling (GL_TEXTURE_MAG_FILTER) | filtering type
	// Sets to use linear interpolation between adjacent mipmaps
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Sets to use linear interpolation upscaling (past largest mipmap texture) (default upscaling method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load texture and set width, height and colour channel params
	data = stbi_load("media/water.jpg", &width, &height, &colourChannels, 0);

	if (data) // If retrieval was successful
	{
		// Generation of texture from retrieved texture data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		// Automatically generates all required mipmaps on bound texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else // If retrieval unsuccessful
	{
		cout << "Failed to load texture.\n";
		return -1;
	}

	// Clears retrieved texture from memory
	stbi_image_free(data);



	/*******************************************************************************************
	*
	* CREATING LIGHT SOURCE
	*
	*******************************************************************************************/
	// Set index of the VAO.
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCube), verticesCube, GL_STATIC_DRAW);

	// Vertices only for the light
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set up uniform variables for light colour for our regular cube objects, plus light pos
	// for light shader
	GLint objColourLoc = glGetUniformLocation(program, "objColour");
	GLint lightColourLoc = glGetUniformLocation(program, "lightColour");
	GLint lightPosLoc = glGetUniformLocation(program, "lightPos");

	// Create uniform variables for the vertex shaders
	GLint modelIdx = glGetUniformLocation(program, "model");
	GLint viewIdx = glGetUniformLocation(program, "view");
	GLint projIdx = glGetUniformLocation(program, "projection");

	GLuint modelLightIdx = glGetUniformLocation(lightProg, "model");
	GLuint viewLightIdx = glGetUniformLocation(lightProg, "view");
	GLuint projLightIdx = glGetUniformLocation(lightProg, "projection");

	GLuint viewPosIdx = glGetUniformLocation(program, "viewPos");


	// Unbind all
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// For rectangle
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	/*******************************************************************************************
	* 
	*	**** RENDER LOOP ****
	*	---------------------
	* 
	*******************************************************************************************/

	// This is the render loop. glfwWindowShouldClose() is always false (by default)
	// until the 'X' is clicked on the window. It can also manually be changed, see 
	// ProcessUserInput().

	/*glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "textureInSand"), 0);
	glUniform1i(glGetUniformLocation(program, "textureInGrass"), 1);
	glUniform1i(glGetUniformLocation(program, "textureInWater"), 2);*/
	terrainShaders.use();
	terrainShaders.setInt("textureInSand", 0);
	terrainShaders.setInt("textureInGrass", 1);
	terrainShaders.setInt("textureInWater", 2);

	while (!glfwWindowShouldClose(window))
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// *** Time & Frames *** //
		// --------------------- //
		float currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		//glUseProgram(program);

		//// Set current light position for cube fragment shader to handle cube lighting
		//glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);

		//// Set current camera position for specular lighting
		//glUniform3f(viewPosIdx, cameraPos.x, cameraPos.y, cameraPos.z);
		terrainShaders.use();
		terrainShaders.setVec3("lightPos", lightPos);
		terrainShaders.setVec3("viewPos", cameraPos);


		/////////////////////////////////////////////////////////////////////////////////////
		// *** Process user input *** //
		// -------------------------- //
		ProcessUserInput(window);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Render *** //
		// -------------- //
		// The colour to be displayed on the cleared window (RGBA)

		glClearColor(currSkyColour.x, currSkyColour.y, currSkyColour.z, 1.0f);

		// Clears the colour buffer. This is necessary to apply the colour we want.
		// | GL_DEPTH_BUFFER_BIT allows for textures to have depth perception - otherwise
		// they warp strangely
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Transform *** //
		// ----------------- //
		// Dynamic transformations must be done in the rendering loop for constant updates.

		/*** (Model-view-projection) ***/

		// Set our view for the MVP here.
		// Initialise the view & relative positioning.
		// First param  - position
		// Second param - directional rotation - this is our default camera world pos + the front (-1 on z axis),
		//				  giving forward camera movement
		// Third param  - up vector (absolute up direction, setting -1.0f would flip the world, we have ours at y = 1.0f)
		view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = perspective(radians(45.0f), (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width / (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height, 0.1f, 100.0f);


		// Set value of colour and light colour
		/*glUniform3f(objColourLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColourLoc, 1.0f, 1.0f, 1.0f);*/
		terrainShaders.setVec3("objColour", 1.0f, 1.0f, 1.0f);
		terrainShaders.setVec3("lightColour", 1.0f, 1.0f, 1.0f);



		// Create matrix, same as transform before
		model = mat4(1.0f);

		// Look straight forward
		model = rotate(model, radians(0.0f), vec3(1.0f, 0.0f, 0.0f));

		model = translate(model, terrainStart);

		// Set our MVP matrix, mvp, to the uniform variable
		/*glUniformMatrix4fv(modelIdx, 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(viewIdx, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(projIdx, 1, GL_FALSE, value_ptr(projection));*/
		terrainShaders.setMat4("model", model);
		terrainShaders.setMat4("view", view);
		terrainShaders.setMat4("projection", projection);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw *** //
		// ------------ //;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE1]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE2]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, buffers[TEXTURE3]);
		glBindVertexArray(VAOs[0]); // Bind buffer object to render; VAOs[0]

		glDrawElements(GL_TRIANGLES, MAP_SIZE * 32, GL_UNSIGNED_INT, 0);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw Rock *** //
		// ----------------- //;
		// Create matrix, same as transform before
		modelShaders.use();

		modelShaders.setVec3("lightPos", lightPos);
		modelShaders.setVec3("objColour", 1.0f, 1.0f, 1.0f);
		modelShaders.setVec3("lightColour", 1.0f, 1.0f, 1.0f);

		model = mat4(1.0f);

		vec3 rockPos = terrainStart;

		// For now, offset by randomly chosen vertice on the terrain to place it somewhere
		rockPos.x += terrainVertices[167522][0];
		rockPos.y += terrainVertices[167522][1] + 0.05f;
		rockPos.z += terrainVertices[167522][2];

		model = translate(model, rockPos);

		// Scale down rock object
		model = scale(model, vec3(0.0025f, 0.0025f, 0.0025f));


		// Set our MVP matrix, mvp, to the uniform variable
		modelShaders.setMat4("model", model);
		modelShaders.setMat4("view", view);
		modelShaders.setMat4("projection", projection);
		modelShaders.setVec3("viewPos", cameraPos);

		// Draw model using enabled shaders
		rock.Draw(modelShaders);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw the Light Cube *** //
		// --------------------------- //
		// Switch to light cube shaders, bind other VAO
		//glUseProgram(lightProg);
		lightShaders.use();
		glBindVertexArray(lightVAO);

		model = mat4(1.0f);

		// Move model
		model = translate(model, lightPos);

		// Set our MVP matrix, mvp, to the uniform variable
		/*glUniformMatrix4fv(modelLightIdx, 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(viewLightIdx, 1, GL_FALSE, value_ptr(view));
		glUniformMatrix4fv(projLightIdx, 1, GL_FALSE, value_ptr(projection));*/
		lightShaders.setMat4("model", model);
		lightShaders.setMat4("view", view);
		lightShaders.setMat4("projection", projection);

		// Draw
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Move light position for next time
		moveLight(glfwGetTime());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Refresh *** //
		// --------------- //
		// Constantly swap colour buffer being rendered to the window.
		//	- The colour buffer is a 2D buffer containing colour values per pixel of
		//	  the window it's being rendered to.
		glfwSwapBuffers(window);

		// Constantly query whether any GLFW events have been triggered - such as
		// the frameBufferSizeCallback() function
		glfwPollEvents();
	}

	/*******************************************************************************************
	*
	* TERMINATE
	*
	*******************************************************************************************/

	// Safely terminate GLFW
	glfwTerminate();

	return 0;
}

// Rotate the light around the scene & adjust the skybox colour based on the light's position
void moveLight(double currTime)
{
	// Light positions for each stage of day/night cycle
	// Midday
	float day1X = MIDDLE_POS;
	float day1Y = MIDDLE_POS + LIGHT_ORBIT_OFFSET;
	// Sunset
	float day2X = START_POS - LIGHT_ORBIT_OFFSET;
	float day2Y = 0.0f;
	// Midnight
	float day3X = MIDDLE_POS;
	float day3Y = -MIDDLE_POS - LIGHT_ORBIT_OFFSET;
	// Sunrise
	float day4X = END_POS + LIGHT_ORBIT_OFFSET;
	float day4Y = 0.0f;

	static vec3 lastSkyColour = day1;

	vec3 colourDiff, newColour;

	float tmaxX, tmaxY, tmaxZ, tminX, tminY, tminZ, rmax, rmin, currentDist;

	// Inflate radius so the light source can rotate around the centre point but remain outside of the actual terrain
	float radius = MIDDLE_POS + 8.0f;

	float centreX = MIDDLE_POS;
	float centreY = 0.0f;

	float x = centreX - radius * sin(currTime * 0.25f);
	float y = centreY + radius * cos(currTime * 0.25f);

	lightPos.x = x;
	lightPos.y = y;

	if ((int)x == (int)day1X && (int)y == (int)day1Y)
	{
		currSkyColour = day1;
		lastSkyColour = day1;
	}
	else if ((int)x == (int)day2X && (int)y == (int)day2Y)
	{
		currSkyColour = day2;
		lastSkyColour = day2;
	}
	else if ((int)x == (int)day3X && (int)y == (int)day3Y)
	{
		currSkyColour = day3;
		lastSkyColour = day3;
	}
	else if ((int)x == (int)day4X && (int)y == (int)day4Y)
	{
		currSkyColour = day4;
		lastSkyColour = day4;
	}
	else
	{
		// Minimum range value
		rmin = 0.0f;

		// Scale the change in colour according to the current light's position between the two major points of
		// sky colour change (e.g. midday -> sunset, sunset -> midnight)
		if (lastSkyColour == day1)
		{
			colourDiff = day2 - day1;

			// Diff between last and target positions
			rmax = sqrt(abs(day2X - day1X) * abs(day2X - day1X) + abs(day2Y - day1Y) * abs(day2Y - day1Y));		

			// tmin = minimum value, tmax = maximum value
			tminX = day2.x;
			tmaxX = day1.x;
			tminY = day2.y;
			tmaxY = day1.y;
			tminZ = day2.z;
			tmaxZ = day1.z;

			// Get the current distance between the target position (next point of lighting change) and current position
			currentDist = sqrt(abs(day2X - lightPos.x) * abs(day2X - lightPos.x) + abs(day2Y - lightPos.y) * abs(day2Y - lightPos.y));
		}
		else if (lastSkyColour == day2)
		{
			colourDiff = day3 - day2;

			rmax = sqrt(abs(day3X - day2X) * abs(day3X - day2X) + abs(day3Y - day2Y) * abs(day3Y - day2Y));
			rmin = 0.0f;

			tminX = day3.x;
			tmaxX = day2.x;
			tminY = day3.y;
			tmaxY = day2.y;
			tminZ = day3.z;
			tmaxZ = day2.z;

			currentDist = sqrt(abs(day3X - lightPos.x) * abs(day3X - lightPos.x) + abs(day3Y - lightPos.y) * abs(day3Y - lightPos.y));
		}
		else if (lastSkyColour == day3)
		{
			colourDiff = day4 - day3;

			rmax = sqrt(abs(day4X - day3X) * abs(day4X - day3X) + abs(day4Y - day3Y) * abs(day4Y - day3Y));
			rmin = 0.0f;

			tminX = day4.x;
			tmaxX = day3.x;
			tminY = day4.y;
			tmaxY = day3.y;
			tminZ = day4.z;
			tmaxZ = day3.z;

			currentDist = sqrt(abs(day4X - lightPos.x) * abs(day4X - lightPos.x) + abs(day4Y - lightPos.y) * abs(day4Y - lightPos.y));			
		}
		else if (lastSkyColour == day4)
		{
			colourDiff = day1 - day4;

			rmax = sqrt(abs(day1X - day4X) * abs(day1X - day4X) + abs(day1Y - day4Y) * abs(day1Y - day4Y));
			rmin = 0.0f;

			tminX = day1.x;
			tmaxX = day4.x;
			tminY = day1.y;
			tmaxY = day4.y;
			tminZ = day1.z;
			tmaxZ = day4.z;

			currentDist = sqrt(abs(day1X - lightPos.x) * abs(day1X - lightPos.x) + abs(day1Y - lightPos.y) * abs(day1Y - lightPos.y));
		}

		newColour.x = (currentDist - rmin) / (rmax - rmin) * (tmaxX - tminX) + tminX;
		newColour.y = (currentDist - rmin) / (rmax - rmin) * (tmaxY - tminY) + tminY;
		newColour.z = (currentDist - rmin) / (rmax - rmin) * (tmaxZ - tminZ) + tminZ;

		currSkyColour = newColour;
	}
}

// Callback function to adjust the window width/height for window resizing.
void frameBufferSizeCallback(GLFWwindow* pW, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Callback to handle mouse events
void mouseCallback(GLFWwindow* pW, double x, double y)
{
	float xOffset, yOffset = 0.0f;

	// Get last x/y positions if necessary
	if (mouseFirstEntry)
	{
		cameraLastXPos = (float)x;
		cameraLastYPos = (float)y;

		mouseFirstEntry = false;
	}

	// Sets change value between last frame and current frame
	xOffset = (float)x - cameraLastXPos;
	yOffset = cameraLastYPos - (float)y; // Other way around - inverted y axis controls

	// Set last positions to current in time for the next frame
	cameraLastXPos = (float)x;
	cameraLastYPos = (float)y;

	// Like movement speed, set mouse sensitivity value, and apply to
	// x/y camera pos changes
	const float sensitivity = 0.075f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Readjust camera yaw (L/R) and pitch (U/D) based on x/y position changes
	cameraYaw += xOffset;
	cameraPitch += yOffset;

	// Prevent camera flipping upside down - otherwise this would be infinite
	if (cameraPitch > 89.0f)
	{
		cameraPitch = 89.0f;
	}
	else if (cameraPitch < -89.0f)
	{
		cameraPitch = -89.0f;
	}

	// Set direction vector
	vec3 direction;
	direction.x = cos(radians(cameraYaw)) * cos(radians(cameraPitch));
	direction.y = sin(radians(cameraPitch));
	direction.z = sin(radians(cameraYaw)) * cos(radians(cameraPitch));
	cameraFront = normalize(direction);
}

void ProcessUserInput(GLFWwindow* pW)
{
	// Allows user to press escape to close the window
	if (glfwGetKey(pW, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(pW, true);
	}

	// Calculates movement speed based on time
	float moveSpeed = 3.0f * deltaTime;

	// Shift - run
	if (glfwGetKey(pW, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		moveSpeed *= 4;
	}

	// W - forward
	if (glfwGetKey(pW, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraPos += moveSpeed * cameraFront;
	}

	// S - backward
	if (glfwGetKey(pW, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraPos -= moveSpeed * cameraFront;
	}

	// D - right
	if (glfwGetKey(pW, GLFW_KEY_D) == GLFW_PRESS)
	{
		// Uses cross product using camera front position (-1 on z axis)
		// and camera global up position (currently 1 on y axis) to get
		// the sizeways (x) vector, hence left/right movement.
		//
		// The cross product basically takes any two vectors in a 3D space 
		// and returns what the third vector is.
		cameraPos += normalize(cross(cameraFront, cameraUp)) * moveSpeed;
	}

	// A - left
	if (glfwGetKey(pW, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraPos -= normalize(cross(cameraFront, cameraUp)) * moveSpeed;
	}
}