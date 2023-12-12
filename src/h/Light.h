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

class Light
{
public:
	Light();

	VAO::VertexData* getVertices();

	void moveLight(double currTime);
	vec3 getLightPosition();

	vec3 getSkyColour();



private:
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

	const vec3 day1 = vec3(0.0f, 0.6f, 1.0f); // Midday
	const vec3 day2 = vec3(1.0f, 0.5f, 0.0f); // Sunset
	const vec3 day3 = vec3(0.0f, 0.0f, 0.1f); // Midnight
	const vec3 day4 = vec3(0.9f, 0.0f, 0.2f); // Sunrise

	vec3 currSkyColour;

	vec3 lightPos;
};