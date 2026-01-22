#pragma once
#ifndef UTLITY_H
#define UTLITY_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Transformations///contains all information of the 3 basic transforms
{
	glm::mat4 modeltransform = glm::mat4();
	glm::mat4 viewtransform = glm::mat4();
	glm::mat4 projectiontransform = glm::mat4();
};

struct Point_Data //contains all the main information for all points 
{
	float PVv = 0, Cov = 0, Spv = 0;
};

double Getworldposfrommouse(GLFWwindow* window, float zoomlevel,float roundinglevel, double &cursorx, double &cursory, float camx, float monitorwidth, float monitorheight,float monitoroffset);
int findcount(float timestep, float currentpointpos, double input);
#endif