#include "Buffers.h"

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define NUM_FACES			6
#define TRIANGLES_PER_FACE	6
#define NUM_COORDS			3
#define NUM_LIGHT_VERTICES	NUM_FACES * TRIANGLES_PER_FACE * NUM_COORDS

using namespace glm;

// Class for creating a movable light source
class Light
{
public:
	vec3 lightColour;

	Light();

	void moveLight(double currTime);
	vec3 getLightPosition();

	vec3 getSkyColour();
	vec3 getLightColour();

	void createLightVAO();
	void drawLight();
private:

	VAO* lightVAO;

	// Cube vertices - light source
	VAO::VertexData verticesCube[NUM_LIGHT_VERTICES] =
	{
		// Positions (vertices only)	
		{ vec3(-0.5f, -0.5f, -0.5f) }, // Face 1, triangle 1
		{ vec3(0.5f, -0.5f, -0.5f)  },
		{ vec3(0.5f,  0.5f, -0.5f)  },
		{ vec3(0.5f,  0.5f, -0.5f)  }, // Face 1, triangle 2
		{ vec3(-0.5f,  0.5f, -0.5f) },
		{ vec3(-0.5f, -0.5f, -0.5f) },

		{ vec3(-0.5f, -0.5f,  0.5f) }, // Face 2
		{ vec3(0.5f, -0.5f,  0.5f)	},
		{ vec3(0.5f,  0.5f,  0.5f)	},
		{ vec3(0.5f,  0.5f,  0.5f)	},
		{ vec3(-0.5f,  0.5f,  0.5f)	},
		{ vec3(-0.5f, -0.5f,  0.5f)	},

		{ vec3(-0.5f,  0.5f,  0.5f)	}, // Face 3... etc.
		{ vec3(-0.5f,  0.5f, -0.5f)	},
		{ vec3(-0.5f, -0.5f, -0.5f)	},
		{ vec3(-0.5f, -0.5f, -0.5f)	},
		{ vec3(-0.5f, -0.5f,  0.5f)	},
		{ vec3(-0.5f,  0.5f,  0.5f)	},

		{ vec3(0.5f,  0.5f,  0.5f)	},
		{ vec3(0.5f,  0.5f, -0.5f)	},
		{ vec3(0.5f, -0.5f, -0.5f)	},
		{ vec3(0.5f, -0.5f, -0.5f)	},
		{ vec3(0.5f, -0.5f,  0.5f)	},
		{ vec3(0.5f,  0.5f,  0.5f)	},

		{ vec3(-0.5f, -0.5f, -0.5f)	},
		{ vec3(0.5f, -0.5f, -0.5f)	},
		{ vec3(0.5f, -0.5f,  0.5f)	},
		{ vec3(0.5f, -0.5f,  0.5f)	},
		{ vec3(-0.5f, -0.5f,  0.5f)	},
		{ vec3(-0.5f, -0.5f, -0.5f)	},

		{ vec3(-0.5f,  0.5f, -0.5f)	},
		{ vec3(0.5f,  0.5f, -0.5f)	},
		{ vec3(0.5f,  0.5f,  0.5f)	},
		{ vec3(0.5f,  0.5f,  0.5f)	},
		{ vec3(-0.5f,  0.5f,  0.5f)	},
		{ vec3(-0.5f,  0.5f, -0.5f)	}
	};

	// Sky colours
	const vec3 day1 = vec3(0.0f, 0.6f, 1.0f); // Midday
	const vec3 day2 = vec3(1.0f, 0.5f, 0.0f); // Sunset
	const vec3 day3 = vec3(0.0f, 0.0f, 0.1f); // Midnight
	const vec3 day4 = vec3(0.9f, 0.0f, 0.2f); // Sunrise

	// Lighting colours
	const vec3 day1L = vec3(1.0f);				// Midday
	const vec3 day2L = vec3(1.0f, 0.8f, 0.3f);	// Sunset
	const vec3 day3L = vec3(0.0f);				// Midnight
	const vec3 day4L = vec3(1.0f, 0.6f, 0.0f);	// Sunrise

	vec3 currSkyColour;

	vec3 lightPos;
};