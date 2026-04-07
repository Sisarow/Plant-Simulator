#pragma once
#ifndef OBJECT_H
#define OBJECT_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Object.h" //used for other classes like graph or backround objects or other rendered stuff which is less important.
#include "Utility.h"
#include <vector>

class Graph
{
	unsigned int VAO, VBO, instanceVBO;
	int num_of_points_s;
	glm::vec3 graphboxpos[220];
public:
	Graph(float Veritces[], int num_of_points,int size);
	void render(const Shader&, const Transformations&);
private:

};


class plotline
{
	unsigned int VAO, VBO;
	bool reachedmax = false;
	glm::vec3 color{ 0.0,0.0,0.0 };
	float Total_time = 0; //Total absolute time elapsed.
	unsigned long long int Maximum_graphed_floats = 0; // maximum floats graphed at once.
public:
	unsigned long long int point_pos_step = 0; // point id at a specific update;
	unsigned long long int pointcount = 0;
	plotline(unsigned long long int max_num_of_points, glm::vec3 color);
	plotline(unsigned long long int max_num_of_points, float color_red, float color_green, float color_blue);
	void render(const Shader&, const Transformations&, float timestep, float input_value, float index_point_value);
	void render(const Shader&, const Transformations&);
	void Update(float timestep, float input_value, float index_point_value);
private:
};


class Flag
{
	unsigned int VAO, VBO;
	int num_of_points_s;
public:
	Flag(glm::vec3 color);
	void render(const Shader&, const Transformations&);
private:
};

class Plane
{
	unsigned int VAO, VBO;
	int num_of_points_s;
public:
	Plane();
	void render(const Shader&, const Transformations&);

private:

};


#endif
