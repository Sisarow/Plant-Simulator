#pragma once
#ifndef RENDER_H
#define RENDER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Bind_Render(unsigned int VAO, unsigned int VBO,int Buffer_Data_size,float vertices[], int Points_per_vertice); // builds the standard VAO/VBO rendering pipeline. Only Works for float vertices and assumes 3 point spacing.
void Bind_Instanced_VBO(unsigned int Instanced_VBO, int Buffer_Data_size,int Passed_in_data[], int Vertex_atribute_location); //builds an instanced VBO for rendering instanced objects. Assumes 3 point spacing with floats.

#endif // !RENDER_H

/************************
This Header file is work in progress and I decided that I will not be finishing it since it is not nessasry for this current project.
*************************/