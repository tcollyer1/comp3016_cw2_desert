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
#include <math.h>

#include "..\h\main.h"
#include "..\h\Display.h"
#include "..\h\Buffers.h"
#include "..\h\Terrain.h"
#include "..\h\Light.h"

using namespace std;
using namespace glm;

GLuint program;
GLuint lightProg;

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

const vec3 terrainStart = vec3(0.0f, -2.0f, -1.5f);

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
	Display* d = new Display(mouseCallback, frameBufferSizeCallback);

	if (d->checkErrors())
	{
		cout << "\nErrors found: closing program.\n";

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

	/*******************************************************************************************
	* 
	* --- MAIN CODE ---
	* 
	*******************************************************************************************/

	/*******************************************************************************************
	*
	* CREATING THE TERRAIN VAO FOR VERTICES AND COLOURS
	*
	*******************************************************************************************/

	Terrain* terrain = new Terrain();

	VAO* terrainVAO = new VAO();
	terrainVAO->bind();

	int verticesArrSize = sizeof(*terrain->getVertices()) * MAP_SIZE;
	int indicesArrSize = sizeof(*terrain->getIndices()) * TOTAL_TRIANGLES;

	// Bind terrain vertices and indices to buffers
	terrainVAO->addBuffer(terrain->getVertices(), verticesArrSize, VAO::VERTICES);
	terrainVAO->addBuffer(terrain->getIndices(), indicesArrSize, VAO::INDICES);

	terrainVAO->enableAttribArrays();

	terrainVAO->unbind();

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
	VAO* testLightVAO = new VAO();
	testLightVAO->bind();

	Light* light = new Light();

	verticesArrSize = sizeof(*light->getVertices()) * NUM_LIGHT_VERTICES;
	testLightVAO->addBuffer(light->getVertices(), verticesArrSize, VAO::VERTICES);

	testLightVAO->enableAttribArrays();

	testLightVAO->unbind();

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

	while (!glfwWindowShouldClose(d->getWindow()))
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
		ProcessUserInput(d->getWindow());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Render *** //
		// -------------- //
		// The colour to be displayed on the cleared window (RGBA)

		glClearColor(light->getSkyColour().x, light->getSkyColour().y, light->getSkyColour().z, 1.0f);

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
		//glBindVertexArray(VAOs[0]); // Bind buffer object to render; VAOs[0]
		terrainVAO->bind();

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
		/*rockPos.x += terrainVertices[167522][0];
		rockPos.y += terrainVertices[167522][1] + 0.05f;
		rockPos.z += terrainVertices[167522][2];*/
		rockPos.x += terrain->getVertices()[TOP_RIGHT].vertices.x;
		rockPos.y += terrain->getVertices()[TOP_RIGHT].vertices.y;
		rockPos.z += terrain->getVertices()[TOP_RIGHT].vertices.z;

		model = translate(model, rockPos);

		// Scale down rock object
		model = scale(model, vec3(0.025f, 0.025f, 0.025f));


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
		//glBindVertexArray(lightVAO);
		testLightVAO->bind();

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
		light->moveLight(glfwGetTime(), &lightPos);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Refresh *** //
		// --------------- //
		// Constantly swap colour buffer being rendered to the window.
		//	- The colour buffer is a 2D buffer containing colour values per pixel of
		//	  the window it's being rendered to.
		glfwSwapBuffers(d->getWindow());

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