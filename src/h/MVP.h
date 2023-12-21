#ifndef MVP_H

#define MVP_H

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>


using namespace glm;

class MVP
{
private:
	// Global matrices for transformations
	mat4 transform;
	mat4 model;
	mat4 view;
	mat4 projection;

public:
	MVP();

	void resetModel();
	void moveModel(vec3 position);
	void scaleModel(vec3 scalingFactor);
	void rotateModel(float degrees, vec3 rotationVec);

	void updateView(vec3 eye, vec3 centre, vec3 up);

	void setProjection();

	mat4 getModel();
	mat4 getView();
	mat4 getProjection();
};

#endif