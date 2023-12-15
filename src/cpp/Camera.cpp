#include "..\h\Camera.h"
//#include "..\h\Terrain.h"

Camera::Camera(Terrain* t)
{
	//camInfo.cameraPos	= vec3(0.0f, 0.0f, 3.0f);
	// Set user (camera) position to start of the terrain
	// + VERTICE_OFFSET for x/z so not directly at the edge
	// + USER_HEIGHT for y so the user is not crawling or clipping through the ground
	camInfo.cameraPos	= vec3(TERRAIN_START.x + VERTICE_OFFSET, TERRAIN_START.y + USER_HEIGHT, TERRAIN_START.z + VERTICE_OFFSET);
	camInfo.cameraFront = vec3(0.0f, 0.0f, -1.0f);
	camInfo.cameraUp	= vec3(0.0f, 1.0f, 0.0f);

	cameraYaw			= -90.0f;
	cameraPitch			= 0.0f;

	// If it's first entry, it has no previous positions so must set them
	mouseFirstEntry		= true;

	cameraLastXPos		= 0.0f;
	cameraLastYPos		= 0.0f;

	terrain				= t;
	mode				= WALK;
}

Camera::CameraInfo Camera::getCameraInfo()
{
	return (camInfo);
}

// Callback to handle mouse events
void Camera::mouseCallback(GLFWwindow* pW, double x, double y)
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
	camInfo.cameraFront = normalize(direction);
}

// Handles camera movement based on user's input. As most controls influence
// camera position and direction, this is in the Camera class
void Camera::processUserInput(GLFWwindow* pW, float deltaTime)
{
	vec3 actualPos = camInfo.cameraPos;
	vec3 actualFront = camInfo.cameraFront;

	vec3 proposedPos = camInfo.cameraPos;

	// Don't move up or down when walking
	actualFront.y = 0.0f;

	bool atEdge = false;

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
		moveSpeed *= 2;
	}

	// Toggle fly/walk modes
	if (glfwGetKey(pW, GLFW_KEY_V) == GLFW_PRESS)
	{
		toggleFly();
	}
	if (glfwGetKey(pW, GLFW_KEY_B) == GLFW_PRESS)
	{
		toggleWalk();
	}

	// W - forward
	if (glfwGetKey(pW, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (mode == WALK)
		{
			// TODO: only move according to terrain height (no flying):
			terrain->offsetUserPos(&actualPos);

			proposedPos.y = TERRAIN_START.y + USER_HEIGHT + actualPos.y;
			proposedPos += moveSpeed * actualFront;

			if (!terrain->isAtEdge(proposedPos))
			{
				camInfo.cameraPos = proposedPos;
			}
		}

		else
		{
			camInfo.cameraPos += moveSpeed * camInfo.cameraFront;
		}
	}

	// S - backward
	if (glfwGetKey(pW, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (mode == WALK)
		{
			terrain->offsetUserPos(&actualPos);

			proposedPos.y = TERRAIN_START.y + USER_HEIGHT + actualPos.y;
			proposedPos -= moveSpeed * actualFront;

			if (!terrain->isAtEdge(proposedPos))
			{
				camInfo.cameraPos = proposedPos;
			}
		}

		else
		{
			camInfo.cameraPos -= moveSpeed * camInfo.cameraFront;
		}
	}

	// D - right
	if (glfwGetKey(pW, GLFW_KEY_D) == GLFW_PRESS)
	{
		// Uses cross product using camera front position and camera global 
		// up position to get the sizeways (x) vector, hence left/right movement.

		if (mode == WALK)
		{
			terrain->offsetUserPos(&actualPos);

			proposedPos.y = TERRAIN_START.y + USER_HEIGHT + actualPos.y;
			//camInfo.cameraPos += normalize(cross(camInfo.cameraFront, camInfo.cameraUp)) * moveSpeed;
			proposedPos += normalize(cross(actualFront, camInfo.cameraUp)) * moveSpeed;

			if (!terrain->isAtEdge(proposedPos))
			{
				camInfo.cameraPos = proposedPos;
			}
		}

		else
		{
			camInfo.cameraPos += normalize(cross(camInfo.cameraFront, camInfo.cameraUp)) * moveSpeed;
		}
	}

	// A - left
	if (glfwGetKey(pW, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (mode == WALK)
		{
			terrain->offsetUserPos(&actualPos);

			proposedPos.y = TERRAIN_START.y + USER_HEIGHT + actualPos.y;
			proposedPos -= normalize(cross(actualFront, camInfo.cameraUp)) * moveSpeed;

			if (!terrain->isAtEdge(proposedPos))
			{
				camInfo.cameraPos = proposedPos;
			}
		}

		else
		{
			camInfo.cameraPos -= normalize(cross(camInfo.cameraFront, camInfo.cameraUp)) * moveSpeed;
		}
	}
}

void Camera::toggleFly()
{
	mode = FLY;
}

void Camera::toggleWalk()
{
	if (mode != WALK)
	{
		// Reset camera pos to start of terrain
		camInfo.cameraPos = vec3(TERRAIN_START.x + VERTICE_OFFSET, TERRAIN_START.y + USER_HEIGHT, TERRAIN_START.z + VERTICE_OFFSET);
	}

	mode = WALK;
}