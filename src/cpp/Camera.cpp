#include "..\h\Camera.h"

Camera::Camera()
{
	camInfo.cameraPos	= vec3(0.0f, 0.0f, 3.0f);
	camInfo.cameraFront = vec3(0.0f, 0.0f, -1.0f);
	camInfo.cameraUp	= vec3(0.0f, 1.0f, 0.0f);

	cameraYaw			= -90.0f;
	cameraPitch			= 0.0f;

	// If it's first entry, it has no previous positions so must set them
	mouseFirstEntry		= true;

	cameraLastXPos		= 0.0f;
	cameraLastYPos		= 0.0f;
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
		camInfo.cameraPos += moveSpeed * camInfo.cameraFront;
	}

	// S - backward
	if (glfwGetKey(pW, GLFW_KEY_S) == GLFW_PRESS)
	{
		camInfo.cameraPos -= moveSpeed * camInfo.cameraFront;
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
		camInfo.cameraPos += normalize(cross(camInfo.cameraFront, camInfo.cameraUp)) * moveSpeed;
	}

	// A - left
	if (glfwGetKey(pW, GLFW_KEY_A) == GLFW_PRESS)
	{
		camInfo.cameraPos -= normalize(cross(camInfo.cameraFront, camInfo.cameraUp)) * moveSpeed;
	}
}