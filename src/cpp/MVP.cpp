#include "..\h\MVP.h"


MVP::MVP()
{
	model = mat4(1.0f);
	view = mat4(1.0f);
	projection = mat4(1.0f);
	transform = mat4(1.0f);
}

void MVP::resetModel()
{
	model = mat4(1.0f);
}

void MVP::updateView(vec3 eye, vec3 centre, vec3 up)
{
	view = lookAt(eye, centre, up);
}

void MVP::setProjection()
{
	projection = perspective(radians(45.0f), (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->width / (float)glfwGetVideoMode(glfwGetPrimaryMonitor())->height, 0.1f, 100.0f);
}

void MVP::moveModel(vec3 position)
{
	model = translate(model, position);
}

mat4 MVP::getModel()
{
	return (model);
}

mat4 MVP::getView()
{
	return (view);
}

mat4 MVP::getProjection()
{
	return (projection);
}