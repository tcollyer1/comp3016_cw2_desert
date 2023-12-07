//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Light
{
public:
	Light();

private:
	const vec3 day1 = vec3(0.0f, 0.6f, 1.0f); // Midday
	const vec3 day2 = vec3(1.0f, 0.5f, 0.0f); // Sunset
	const vec3 day3 = vec3(0.0f, 0.0f, 0.1f); // Midnight
	const vec3 day4 = vec3(0.9f, 0.0f, 0.2f); // Sunrise

	vec3 currSkyColour;

	const vec3 terrainStart = vec3(0.0f, -2.0f, -1.5f);
};