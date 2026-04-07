#pragma once
#ifndef SIMVIEW_H
#define SIMVIEW_H
/************
This file is used to define the plane sim view child window and functions for a seperate thread. 
If the plane simulator is chosen to run only.
**************/
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <shader.h>//custom header file for easy shader creation and use.
#include "Controller.h" //used for creating and running the controller 
#include "processsim.h"//used for creating a process.
#include "Object.h" //used for other classes like graph or backround objects or other rendered stuff which is less important.
#include "Utility.h" // used for rendering stuffs. also contains some usefull fuctions and stucts.
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>


extern Processsim* ProcessPTR;
extern Controller* controlPTR;
extern Controller* controlPTR2;
extern GLFWwindow* simview_window;
extern bool childshouldclose;
extern int monitorWidth, monitorHeight;


void Child_window();
void framebuffer_size_callbacksim(GLFWwindow*, int, int);

#endif