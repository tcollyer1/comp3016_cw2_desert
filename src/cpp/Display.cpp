#include "..\h\Display.h"

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

Display::Display(GLFWcursorposfun mouseCallback, GLFWframebuffersizefun frameBuffSizeCallback)
{
	error = false;

	// Initialise GLFW - used for user I/O, creating windows and more using OpenGL
	glfwInit();

	// Create a GLFWwindow window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Desert", NULL, NULL);
	/*window = glfwCreateWindow(glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
		glfwGetVideoMode(glfwGetPrimaryMonitor())->height, "Desert",
		glfwGetPrimaryMonitor(), NULL);*/

	// Checks for successful window creation
	if (window == NULL)
	{
		cout << "GLFW window could not instantiate\n";
		glfwTerminate();

		error = true;
	}

	// Set cursor to bind to window, hides cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Assign window to current OpenGL context
	glfwMakeContextCurrent(window);

	// Initialise GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialise GLAD\n";

		error = true;
	}

	// Width and height should match that of the window
	// 0, 0 indicates the position of the window
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Enables "depth" in textures - stops lack of depth in 3D objects when
	// texturing & strange warping
	glEnable(GL_DEPTH_TEST);

	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND); // Enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Assigns frameBufferSizeCallback() as the callback function to dynamically handle 
	// window resizing.
	glfwSetFramebufferSizeCallback(window, frameBuffSizeCallback);

	// Set mouse callback for mouse movement
	glfwSetCursorPosCallback(window, mouseCallback);
}

GLFWwindow* Display::getWindow()
{
	return (window);
}

bool Display::checkErrors()
{
	return (error);
}