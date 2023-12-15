//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
};