#include <iostream>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shaders
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Other
#include <math.h>
#include <vector>

#include "..\h\main.h"
#include "..\h\Display.h"
#include "..\h\Terrain.h"
#include "..\h\Light.h"
#include "..\h\Camera.h"
#include "..\h\MVP.h"

#define TREE_MAX 0.005f
#define CACTUS_MAX 0.005f
#define GRASS_MAX 0.01f

using namespace std;
using namespace glm;

/*******************************************************************************************
*
* MATRIX GLOBALS
*
*******************************************************************************************/
// Global matrices for transformations
mat4 model;
mat4 view;
mat4 projection;

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
* CAMERA
*
*******************************************************************************************/
Camera* camera = NULL;

int main()
{
	Display* d = new Display(mouseCallback, frameBufferSizeCallback);

	if (d->checkErrors())
	{
		cout << "\nErrors found: closing program.\n";

		return -1;
	}

	// Set shaders & model
	//Shader terrainShaders("shaders/terrainShader.vert", "shaders/terrainShader.frag");
	Shader lightShaders("shaders/lightShader.vert", "shaders/lightShader.frag");
	Shader modelShaders("shaders/modelShader.vert", "shaders/modelShader.frag");

	Model shrub("media/grass/scene.gltf");
	Model palmTree("media/palmTree/CordylineFREE.obj");
	Model cactus("media/cactus/scene.gltf");

	//Terrain* terrain = new Terrain(&terrainShaders);
	Terrain* terrain = new Terrain();
	camera = new Camera(terrain);
	Light* light = new Light();

	vec3 modelPos = vec3(0.0f);


	vector<vec3> grassModPos;
	vector<vec3> oasisModPos;
	terrain->getGrassModelPositions(&grassModPos);
	terrain->getOasisModelPositions(&oasisModPos);

	/*******************************************************************************************
	* 
	*	**** RENDER LOOP ****
	*	---------------------
	* 
	*******************************************************************************************/

	Camera::CameraInfo camInfo;

	MVP* mvp = new MVP(); // Create MVP matrix

	// This is the render loop.
	// until the 'X' is clicked on the window or Esc is hit.

	while (!glfwWindowShouldClose(d->getWindow()))
	{
		/////////////////////////////////////////////////////////////////////////////////////
		// *** Time & Frames *** //
		// --------------------- //
		float currFrame = static_cast<float>(glfwGetTime());
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;

		// Set current camera/light position for specular lighting
		terrain->setShaderPositions(light->getLightPosition(), camera->getCameraInfo().cameraPos);
		//terrainShaders.use();
		//terrainShaders.setVec3("lightPos", light->getLightPosition());
		//terrainShaders.setVec3("viewPos", camera->getCameraInfo().cameraPos);


		/////////////////////////////////////////////////////////////////////////////////////
		// *** Process user input *** //
		// -------------------------- //
		camera->processUserInput(d->getWindow(), deltaTime);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Render *** //
		// -------------- //
		// The colour to be displayed on the cleared window (RGBA)

		glClearColor(light->getSkyColour().r, light->getSkyColour().g, light->getSkyColour().b, 1.0f);

		// Clears the colour buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Transform *** //
		// ----------------- //
		// Dynamic transformations must be done in the rendering loop for constant updates.

		/*** (Model-view-projection) ***/

		// Set our view for the MVP here.
		// Initialise the view & relative positioning.
		camInfo = camera->getCameraInfo();

		//view = lookAt(camInfo.cameraPos, camInfo.cameraPos + camInfo.cameraFront, camInfo.cameraUp);
		mvp->updateView(camInfo.cameraPos, camInfo.cameraPos + camInfo.cameraFront, camInfo.cameraUp);

		//projection = perspective(radians(45.0f), (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width / (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height, 0.1f, 100.0f);
		mvp->setProjection();

		// Create matrix, same as transform before
		//model = mat4(1.0f);
		mvp->resetModel();

		//model = translate(model, TERRAIN_START);
		mvp->moveModel(TERRAIN_START);

		// Set our MVP matrix, mvp, to the uniform variable
		//terrainShaders.setMat4("model", model);
		//terrainShaders.setMat4("view", view);
		//terrainShaders.setMat4("projection", projection);
		terrain->setMVP(mvp);

		// Set value of model colour and light colour
		terrain->setShaderLightColour(light->getLightColour());

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw *** //
		// ------------ //

		// Draw terrain
		terrain->drawTerrain();

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw Models *** //
		// ------------------- //;
		// Create matrix, same as transform before
		modelShaders.use();

		modelShaders.setVec3("lightPos", light->getLightPosition());
		modelShaders.setVec3("lightColour", light->getLightColour());

		// Draw grass biome models (grass, cacti)
		for (int i = 0; i < grassModPos.size(); i++)
		{
			model = mat4(1.0f);

			modelPos.x = TERRAIN_START.x + grassModPos[i].x;
			modelPos.y = TERRAIN_START.y + grassModPos[i].y;
			modelPos.z = TERRAIN_START.z + grassModPos[i].z;

			model = translate(model, modelPos);

			// Set our MVP matrix to the uniform variables
			modelShaders.setMat4("view", mvp->getView());
			modelShaders.setMat4("projection", mvp->getProjection());
			modelShaders.setVec3("viewPos", camInfo.cameraPos);

			// Draw model using enabled shaders
			if (terrain->getModelType(i))
			{
				// Scale down cactus object, draw cactus
				model = scale(model, vec3((float)(CACTUS_MAX / terrain->getScale(i))));
				model = rotate(model, radians((float)terrain->getRotation(i)), vec3(0.0f, 1.0f, 0.0f));

				modelShaders.setMat4("model", model);

				cactus.Draw(modelShaders);
			}
			else
			{
				// Scale down grass object, draw grass
				model = scale(model, vec3((float)(GRASS_MAX / terrain->getScale(i))));
				model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

				modelShaders.setMat4("model", model);

				shrub.Draw(modelShaders);
			}
			
		}

		// Draw desert oasis biome models (trees)
		for (int i = 0; i < oasisModPos.size(); i++)
		{
			model = mat4(1.0f);

			modelPos.x = TERRAIN_START.x + oasisModPos[i].x;
			modelPos.y = TERRAIN_START.y + oasisModPos[i].y;
			modelPos.z = TERRAIN_START.z + oasisModPos[i].z;

			model = translate(model, modelPos);

			// Set our MVP matrix to the uniform variables
			modelShaders.setMat4("view", mvp->getView());
			modelShaders.setMat4("projection", mvp->getProjection());
			modelShaders.setVec3("viewPos", camInfo.cameraPos);

			if (terrain->getModelType(i))
			{
				// Draw model using enabled shaders			
				// Scale down tree object, draw tree
				model = scale(model, vec3(0.005f));
				model = rotate(model, radians((float)terrain->getRotation(i)), vec3(0.0f, 1.0f, 0.0f));

				modelShaders.setMat4("model", model);

				palmTree.Draw(modelShaders);
			}
			else
			{
				// Scale down grass object, draw grass
				model = scale(model, vec3((float)(GRASS_MAX / terrain->getScale(i))));
				model = rotate(model, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

				modelShaders.setMat4("model", model);

				shrub.Draw(modelShaders);
			}

				
		}

		/////////////////////////////////////////////////////////////////////////////////////
		// *** Draw the Light Cube *** //
		// --------------------------- //
		// Switch to light cube shaders, bind other VAO
		lightShaders.use();

		lightShaders.setVec3("lightColour", light->getLightColour());

		model = mat4(1.0f);

		// Move model
		model = translate(model, light->getLightPosition());

		// Set our MVP matrix to the uniform variables
		lightShaders.setMat4("model", model);
		lightShaders.setMat4("view", mvp->getView());
		lightShaders.setMat4("projection", mvp->getProjection());

		// Draw
		light->drawLight();

		// Move light position for next time
		light->moveLight(glfwGetTime());

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

// Callback function to handle mouse movement.
void mouseCallback(GLFWwindow* pW, double x, double y)
{
	if (camera != NULL)
	{
		camera->mouseCallback(pW, x, y);
	}
	else
	{
		cout << "ERROR: camera is null\n";
	}
}

// Callback function to adjust the window width/height for window resizing.
void frameBufferSizeCallback(GLFWwindow* pW, int width, int height)
{
	glViewport(0, 0, width, height);
}

