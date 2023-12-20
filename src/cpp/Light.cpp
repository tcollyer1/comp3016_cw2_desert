#include "..\h\Light.h"
#include "..\h\Terrain.h"

#define LIGHT_ORBIT_OFFSET	8.0f;

Light::Light()
{
	currSkyColour = day1;
	lightColour = vec3(1.0f);
	lightPos = vec3(MIDDLE_POS, MIDDLE_POS, -MIDDLE_POS);

	createLightVAO();

	engine = createIrrKlangDevice();

	if (!engine)
	{
		cout << "\nError setting up irrKlang engine\n";

		engine	= NULL;
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

vec3 Light::getLightPosition()
{
	return (lightPos);
}

vec3 Light::getSkyColour()
{
	return (currSkyColour);
}

vec3 Light::getLightColour()
{
	return (lightColour);
}

void Light::createLightVAO()
{
	lightVAO = new VAO();
	lightVAO->bind();

	int verticesArrSize = sizeof(verticesCube);
	lightVAO->addBuffer(verticesCube, verticesArrSize, VAO::VERTICES);

	lightVAO->enableAttribArrays(BUF_VERTICES);

	lightVAO->unbind();
}

void Light::drawLight()
{
	lightVAO->bind();

	glDrawArrays(GL_TRIANGLES, 0, 36);

	lightVAO->unbind();
}

// Rotate the light around the scene & adjust the skybox colour based on the light's position
void Light::moveLight(double currTime)
{
	// Light positions for each stage of day/night cycle
	// Midday
	float day1X = MIDDLE_POS;
	float day1Y = MIDDLE_POS + LIGHT_ORBIT_OFFSET;
	// Sunset
	float day2X = START_POS - LIGHT_ORBIT_OFFSET;
	float day2Y = 0.0f;
	// Midnight
	float day3X = MIDDLE_POS;
	float day3Y = -MIDDLE_POS - LIGHT_ORBIT_OFFSET;
	// Sunrise
	float day4X = END_POS + LIGHT_ORBIT_OFFSET;
	float day4Y = 0.0f;

	static vec3 lastSkyColour = day1;

	vec3 newColour, newLightColour;

	float tmaxX, tmaxY, tmaxZ, tminX, tminY, tminZ, rmax, rmin, currentDist;

	float tmaxXL, tmaxYL, tmaxZL, tminXL, tminYL, tminZL, tmaxVol1, tmaxVol2, tminVol1, tminVol2;

	// Inflate radius so the light source can rotate around the centre point but remain outside of the actual terrain
	float radius = MIDDLE_POS + 8.0f;

	float centreX = MIDDLE_POS;
	float centreY = 0.0f;

	float x = centreX - radius * sin(currTime * 0.25f);
	float y = centreY + radius * cos(currTime * 0.25f);

	lightPos.x = x;
	lightPos.y = y;

	if ((int)x == (int)day1X && (int)y == (int)day1Y)
	{
		currSkyColour = day1;
		lastSkyColour = day1;

		if (sound != NULL && sound2 != NULL)
		{
			// Pause night sound
			sound2->setVolume(0.0f);

			// Day sound becomes 100% volume
			sound->setVolume(1.0f);
		}

		lightColour = day1L;
	}
	else if ((int)x == (int)day2X && (int)y == (int)day2Y)
	{
		currSkyColour = day2;
		lastSkyColour = day2;

		if (sound != NULL && sound2 != NULL)
		{
			// Play night sound at 50% volume here
			sound2->setVolume(0.5f);

			// Day sound becomes 50% volume
			sound->setVolume(0.5f);
		}		

		lightColour = day2L;
	}
	else if ((int)x == (int)day3X && (int)y == (int)day3Y)
	{
		currSkyColour = day3;
		lastSkyColour = day3;

		if (sound != NULL && sound2 != NULL)
		{
			// Pause day sound
			sound->setVolume(0.0f);

			// Night sound becomes 100% volume
			sound2->setVolume(1.0f);
		}

		lightColour = day3L;
	}
	else if ((int)x == (int)day4X && (int)y == (int)day4Y)
	{
		currSkyColour = day4;
		lastSkyColour = day4;

		if (sound != NULL && sound2 != NULL)
		{
			// Play day sound at 50% volume here
			sound->setVolume(0.5f);

			// Night sound becomes 50% volume
			sound2->setVolume(0.5f);
		}

		lightColour = day4L;
	}
	else
	{
		// This section performs calculations for the sky colour, light colour and sound volumes

		float lastSoundVal = 0.0f;
		float lastSound2Val = 0.0f;
		float nextSoundVal = 0.0f;
		float nextSound2Val = 0.0f;

		float newVolume1 = 0.0f;
		float newVolume2 = 0.0f;

		// Minimum range value
		rmin = 0.0f;

		// Scale the change in colour according to the current light's position between the two major points of
		// sky colour change (e.g. midday -> sunset, sunset -> midnight)
		if (lastSkyColour == day1)
		{
			lastSoundVal	= 1.0f;
			lastSound2Val	= 0.0f;

			nextSoundVal	= 0.5f;
			nextSound2Val	= 0.5f;

			// Diff between last and target positions
			rmax = sqrt(abs(day2X - day1X) * abs(day2X - day1X) + abs(day2Y - day1Y) * abs(day2Y - day1Y));

			// tmin = minimum value, tmax = maximum value
			tminX = day2.r;
			tmaxX = day1.r;
			tminY = day2.g;
			tmaxY = day1.g;
			tminZ = day2.b;
			tmaxZ = day1.b;

			tminXL = day2L.r;
			tmaxXL = day1L.r;
			tminYL = day2L.g;
			tmaxYL = day1L.g;
			tminZL = day2L.b;
			tmaxZL = day1L.b;

			// Get the current distance between the target position (next point of lighting change) and current position
			currentDist = sqrt(abs(day2X - lightPos.x) * abs(day2X - lightPos.x) + abs(day2Y - lightPos.y) * abs(day2Y - lightPos.y));
		}
		else if (lastSkyColour == day2)
		{
			lastSoundVal	= 0.5f;
			lastSound2Val	= 0.5f;

			nextSoundVal	= 0.0f;
			nextSound2Val	= 1.0f;

			rmax = sqrt(abs(day3X - day2X) * abs(day3X - day2X) + abs(day3Y - day2Y) * abs(day3Y - day2Y));
			rmin = 0.0f;

			tminX = day3.r;
			tmaxX = day2.r;
			tminY = day3.g;
			tmaxY = day2.g;
			tminZ = day3.b;
			tmaxZ = day2.b;

			tminXL = day3L.r;
			tmaxXL = day2L.r;
			tminYL = day3L.g;
			tmaxYL = day2L.g;
			tminZL = day3L.b;
			tmaxZL = day2L.b;

			currentDist = sqrt(abs(day3X - lightPos.x) * abs(day3X - lightPos.x) + abs(day3Y - lightPos.y) * abs(day3Y - lightPos.y));
		}
		else if (lastSkyColour == day3)
		{
			lastSoundVal	= 0.0f;
			lastSound2Val	= 1.0f;

			nextSoundVal	= 0.5f;
			nextSound2Val	= 0.5f;

			rmax = sqrt(abs(day4X - day3X) * abs(day4X - day3X) + abs(day4Y - day3Y) * abs(day4Y - day3Y));
			rmin = 0.0f;

			tminX = day4.r;
			tmaxX = day3.r;
			tminY = day4.g;
			tmaxY = day3.g;
			tminZ = day4.b;
			tmaxZ = day3.b;

			tminXL = day4L.r;
			tmaxXL = day3L.r;
			tminYL = day4L.g;
			tmaxYL = day3L.g;
			tminZL = day4L.b;
			tmaxZL = day3L.b;

			currentDist = sqrt(abs(day4X - lightPos.x) * abs(day4X - lightPos.x) + abs(day4Y - lightPos.y) * abs(day4Y - lightPos.y));
		}
		else if (lastSkyColour == day4)
		{
			lastSoundVal	= 0.5f;
			lastSound2Val	= 0.5f;

			nextSoundVal	= 1.0f;
			nextSound2Val	= 0.0f;

			rmax = sqrt(abs(day1X - day4X) * abs(day1X - day4X) + abs(day1Y - day4Y) * abs(day1Y - day4Y));
			rmin = 0.0f;

			tminX = day1.r;
			tmaxX = day4.r;
			tminY = day1.g;
			tmaxY = day4.g;
			tminZ = day1.b;
			tmaxZ = day4.b;

			tminXL = day1L.r;
			tmaxXL = day4L.r;
			tminYL = day1L.g;
			tmaxYL = day4L.g;
			tminZL = day1L.b;
			tmaxZL = day4L.b;

			currentDist = sqrt(abs(day1X - lightPos.x) * abs(day1X - lightPos.x) + abs(day1Y - lightPos.y) * abs(day1Y - lightPos.y));
		}

		newColour.r = (currentDist - rmin) / (rmax - rmin) * (tmaxX - tminX) + tminX;
		newColour.g = (currentDist - rmin) / (rmax - rmin) * (tmaxY - tminY) + tminY;
		newColour.b = (currentDist - rmin) / (rmax - rmin) * (tmaxZ - tminZ) + tminZ;

		newLightColour.r = (currentDist - rmin) / (rmax - rmin) * (tmaxXL - tminXL) + tminXL;
		newLightColour.g = (currentDist - rmin) / (rmax - rmin) * (tmaxYL - tminYL) + tminYL;
		newLightColour.b = (currentDist - rmin) / (rmax - rmin) * (tmaxZL - tminZL) + tminZL;

		tminVol1 = nextSoundVal;
		tmaxVol1 = lastSoundVal;
		tminVol2 = nextSound2Val;
		tmaxVol2 = lastSound2Val;

		newVolume1 = (currentDist - rmin) / (rmax - rmin) * (tmaxVol1 - tminVol1) + tminVol1;
		newVolume2 = (currentDist - rmin) / (rmax - rmin) * (tmaxVol2 - tminVol2) + tminVol2;

		currSkyColour = newColour;
		lightColour = newLightColour;

		if (sound != NULL && sound2 != NULL)
		{
			sound->setVolume(newVolume1);
			sound2->setVolume(newVolume2);
		}		
	}
}