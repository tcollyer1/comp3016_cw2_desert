//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void frameBufferSizeCallback(GLFWwindow* pW, int width, int height);
void ProcessUserInput(GLFWwindow* pW);
void mouseCallback(GLFWwindow* pW, double x, double y);
void moveLight(double currTime);