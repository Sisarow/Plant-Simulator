#include "SimView.h"

void Child_window()
{
	float widths  = monitorWidth, heights = monitorHeight;
	glfwMakeContextCurrent(simview_window);
	glfwSetFramebufferSizeCallback(simview_window, framebuffer_size_callbacksim);//allows resizing of the window.
	glfwSwapInterval(0);
	Shader shader1s("Vertexshader1.txt", "Fragmentshader1.txt");//creates shaders this one will be used for the graphs and other things.
	Plane plane;

	glm::mat4 transform = glm::mat4(1.0f);//default transform for creating specialized transforms
	glm::mat4 planetransform = glm::mat4();
	glm::mat4 viewtransform = glm::mat4();
	glm::mat4 projectiontransform = glm::mat4();
	//camera stuff below
	glm::vec3 campos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 revcameraDirection = glm::normalize(campos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, revcameraDirection));
	glm::vec3 cameraUp = glm::cross(revcameraDirection, cameraRight);

	planetransform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f)); // model tranform aka position in world relitive to 0.0
	
	viewtransform = glm::lookAt(campos, cameraTarget, up); // view tranform reverced from expected values for directions
	projectiontransform = glm::perspective(glm::radians(45.0f), (float)widths / (float)heights, 0.1f, 100.0f);

	while (!childshouldclose)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		while (!glfwWindowShouldClose(simview_window) && !childshouldclose)
		{

			glClearColor(0.5f, 1.0f, 1.0f, 1.0f);//screen color R, G, B, A
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::vec3 planePos = glm::vec3(ProcessPTR->state.x, ProcessPTR->state.y, ProcessPTR->state.z);
			cameraTarget = { ProcessPTR->state.x, ProcessPTR->state.y, ProcessPTR->state.z };
			glm::vec4 localCameraOffset = glm::vec4(0.0f, 3.0f, 10.0f, 1.0f);
			glm::mat4 planeRot = glm::rotate(glm::mat4(1.0f), -(float)ProcessPTR->state.theta, glm::vec3(1.0f, 0.0f, 0.0f));

			glm::vec3 rotatedOffset = glm::vec3(planeRot * localCameraOffset);

			campos = planePos + glm::vec3(20.0,0.0,2.0);
			glm::vec3 rotatedUp = glm::vec3(planeRot * glm::vec4(up, 0.0f));

			viewtransform = glm::lookAt(campos, cameraTarget, up);
			projectiontransform = glm::perspective(glm::radians(45.0f), (float)widths / (float)heights, 0.1f, 100.0f);
			planetransform = glm::translate(transform, glm::vec3(ProcessPTR->state.x, ProcessPTR->state.y, ProcessPTR->state.z));
			planetransform *= planeRot;
			
			plane.render(shader1s,Transformations{planetransform,viewtransform,projectiontransform});


			//swap buffers and poll IO events (key pressed mouse moved etc)
			glfwSwapBuffers(simview_window);
			std::this_thread::sleep_for(std::chrono::milliseconds(16));

		}

	}

}
void framebuffer_size_callbacksim(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}