#ifndef LIGHT_H

#define LIGHT_H

#include "Buffers.h"
#include "Terrain.h"

#include "ShaderInterface.h"

// Audio
#include <irrKlang/irrKlang.h>

#include <string>


#define NUM_FACES			6
#define TRIANGLES_PER_FACE	6
#define NUM_COORDS			3
#define NUM_LIGHT_VERTICES	NUM_FACES * TRIANGLES_PER_FACE * NUM_COORDS

using namespace std;
using namespace glm;
using namespace irrklang;

// Class for creating a movable light source
class Light : public ShaderInterface
{
public:
	vec3 lightColour;

	Light(string vertexShader, string fragShader, int* err) : ShaderInterface(vertexShader, fragShader, err)
	{
		currSkyColour = day1;
		lightColour = vec3(1.0f);
		lightPos = vec3(MIDDLE_POS, MIDDLE_POS, -MIDDLE_POS);

		createLightVAO();

		engine = createIrrKlangDevice();

		if (!engine)
		{
			cout << "[!] Error setting up irrKlang engine (Light.cpp)\n";

			sound	= NULL;
			sound2	= NULL;
		}
		else
		{
			sound = engine->play2D(daySound.c_str(), true, false, true, ESM_AUTO_DETECT, true);
			sound2 = engine->play2D(nightSound.c_str(), true, false, true, ESM_AUTO_DETECT, true);
			sound2->setVolume(0.0f);
		}
	}

	~Light();

	void moveLight(double currTime);
	void setShaderLightColour();
	vec3 getLightPosition();

	vec3 getSkyColour();
	vec3 getLightColour();

	void createLightVAO();
	void drawLight();
private:

	VAO* lightVAO;

	// Light class controls background ambient sound.
	// (Sound depends on time of day, so in other words
	// the light position)
	ISoundEngine* engine;
	ISound* sound;
	ISound* sound2;

	const string daySound = "media/audio/ambience.mp3";
	const string nightSound = "media/audio/ambience2.mp3";

	// Cube vertices - light source
	const VAO::VertexData verticesCube[NUM_LIGHT_VERTICES] =
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

	// Holds the current sky (background) colour
	vec3 currSkyColour;

	// Holds the current light position
	vec3 lightPos;
};

#endif