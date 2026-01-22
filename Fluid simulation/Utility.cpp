#include "Utility.h"

double Getworldposfrommouse(GLFWwindow* window, float zoomlevel, float roundinglevel, double& cursorx, double& cursory, float camx, float monitorwidth, float monitorheight, float monitoroffset)
{
	glfwGetCursorPos(window, &cursorx, &cursory);//get mouse pos on screen.
	double calculatedvalue;
	calculatedvalue = (((((cursorx / zoomlevel) / (monitorwidth / zoomlevel)) - 0.5)) * (monitorwidth / zoomlevel)) - camx; //get actual world cordinates of mouse click
	calculatedvalue = std::round(calculatedvalue * (1.0f / roundinglevel)); //round to rounding level accuacy.
	calculatedvalue = (calculatedvalue / (1.0f / roundinglevel));
	return calculatedvalue;
}

int findcount(float timestep, float currentpointpos, double input)
{
	int count = (int)std::round(input / timestep);

	if (count > currentpointpos)
	{
		count = currentpointpos;
	}
	if (count < 0)
	{
		count = 0;
	}
	return count;
}

