#include "..\h\Light.h"
#include "..\h\Terrain.h"

#define LIGHT_ORBIT_OFFSET	8.0f;

Light::Light()
{
	currSkyColour = day1;
}

VAO::VertexData* Light::getVertices()
{
	return (verticesCube);
}

vec3 Light::getSkyColour()
{
	return (currSkyColour);
}

// Rotate the light around the scene & adjust the skybox colour based on the light's position
void Light::moveLight(double currTime, vec3* lightPos)
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

	vec3 colourDiff, newColour;

	float tmaxX, tmaxY, tmaxZ, tminX, tminY, tminZ, rmax, rmin, currentDist;

	// Inflate radius so the light source can rotate around the centre point but remain outside of the actual terrain
	float radius = MIDDLE_POS + 8.0f;

	float centreX = MIDDLE_POS;
	float centreY = 0.0f;

	float x = centreX - radius * sin(currTime * 0.25f);
	float y = centreY + radius * cos(currTime * 0.25f);

	lightPos->x = x;
	lightPos->y = y;

	if ((int)x == (int)day1X && (int)y == (int)day1Y)
	{
		currSkyColour = day1;
		lastSkyColour = day1;
	}
	else if ((int)x == (int)day2X && (int)y == (int)day2Y)
	{
		currSkyColour = day2;
		lastSkyColour = day2;
	}
	else if ((int)x == (int)day3X && (int)y == (int)day3Y)
	{
		currSkyColour = day3;
		lastSkyColour = day3;
	}
	else if ((int)x == (int)day4X && (int)y == (int)day4Y)
	{
		currSkyColour = day4;
		lastSkyColour = day4;
	}
	else
	{
		// Minimum range value
		rmin = 0.0f;

		// Scale the change in colour according to the current light's position between the two major points of
		// sky colour change (e.g. midday -> sunset, sunset -> midnight)
		if (lastSkyColour == day1)
		{
			colourDiff = day2 - day1;

			// Diff between last and target positions
			rmax = sqrt(abs(day2X - day1X) * abs(day2X - day1X) + abs(day2Y - day1Y) * abs(day2Y - day1Y));

			// tmin = minimum value, tmax = maximum value
			tminX = day2.x;
			tmaxX = day1.x;
			tminY = day2.y;
			tmaxY = day1.y;
			tminZ = day2.z;
			tmaxZ = day1.z;

			// Get the current distance between the target position (next point of lighting change) and current position
			currentDist = sqrt(abs(day2X - lightPos->x) * abs(day2X - lightPos->x) + abs(day2Y - lightPos->y) * abs(day2Y - lightPos->y));
		}
		else if (lastSkyColour == day2)
		{
			colourDiff = day3 - day2;

			rmax = sqrt(abs(day3X - day2X) * abs(day3X - day2X) + abs(day3Y - day2Y) * abs(day3Y - day2Y));
			rmin = 0.0f;

			tminX = day3.x;
			tmaxX = day2.x;
			tminY = day3.y;
			tmaxY = day2.y;
			tminZ = day3.z;
			tmaxZ = day2.z;

			currentDist = sqrt(abs(day3X - lightPos->x) * abs(day3X - lightPos->x) + abs(day3Y - lightPos->y) * abs(day3Y - lightPos->y));
		}
		else if (lastSkyColour == day3)
		{
			colourDiff = day4 - day3;

			rmax = sqrt(abs(day4X - day3X) * abs(day4X - day3X) + abs(day4Y - day3Y) * abs(day4Y - day3Y));
			rmin = 0.0f;

			tminX = day4.x;
			tmaxX = day3.x;
			tminY = day4.y;
			tmaxY = day3.y;
			tminZ = day4.z;
			tmaxZ = day3.z;

			currentDist = sqrt(abs(day4X - lightPos->x) * abs(day4X - lightPos->x) + abs(day4Y - lightPos->y) * abs(day4Y - lightPos->y));
		}
		else if (lastSkyColour == day4)
		{
			colourDiff = day1 - day4;

			rmax = sqrt(abs(day1X - day4X) * abs(day1X - day4X) + abs(day1Y - day4Y) * abs(day1Y - day4Y));
			rmin = 0.0f;

			tminX = day1.x;
			tmaxX = day4.x;
			tminY = day1.y;
			tmaxY = day4.y;
			tminZ = day1.z;
			tmaxZ = day4.z;

			currentDist = sqrt(abs(day1X - lightPos->x) * abs(day1X - lightPos->x) + abs(day1Y - lightPos->y) * abs(day1Y - lightPos->y));
		}

		newColour.x = (currentDist - rmin) / (rmax - rmin) * (tmaxX - tminX) + tminX;
		newColour.y = (currentDist - rmin) / (rmax - rmin) * (tmaxY - tminY) + tminY;
		newColour.z = (currentDist - rmin) / (rmax - rmin) * (tmaxZ - tminZ) + tminZ;

		currSkyColour = newColour;
	}
}