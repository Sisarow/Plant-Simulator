
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <shader.h>//custom header file for easy shader creation and use.
#include "processsim.h"//used for creating a process.
#include "Utility.h" // used for rendering stuffs. also contains some usefull fuctions and stucts.
#include "Object.h" //used for other classes like graph or backround objects or other rendered stuff which is less important.
#include "Controller.h" //used for creating and running the controller 
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

void framebuffer_size_callback(GLFWwindow*, int, int);
void mousescrollcallback(GLFWwindow*, double, double);
void processInput(GLFWwindow*);
void mouseclickcallback(GLFWwindow*, int, int, int);
void IMGUI_FRAME(int);
int monitorWidth, monitorHeight, monitoroffset = 0;
bool camdetached = false;

const unsigned int max_number_of_points = 4000000000; //defaut is 1 billion points which will make sure memory is not run out during operation.
float timestep = 0.010; // desired time step for desred accuacy of simulation (also can affect simulation speed!). 
float camx = -90, camy = -50, camz = 0, zoom = 10; // for camera stuffs
float deltatime, prevtime; // for finding time between frames.
Processsim* ProcessPTR;
Controller* controlPTR;
std::vector<Point_Data> point;

double cursorposx, cursorposy;

double absoluteflagposf1 = 0;// all input flags which can be controlled with numbers 1-6 on keyboard respectively.
double absoluteflagposf2 = 0;
double absoluteflagposf3 = 0;
double absoluteflagposf4 = 0;
double absoluteflagposf5 = 0;
double absoluteflagposf6 = 0;

int main()
{
	//Setup of Process simulator and Controller.
	Processsim Process1;
	ProcessPTR = &Process1;

	//setup of controller.
	Controller controller1;
	controlPTR = &controller1;

	//window and other things below.
	glfwInit();
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor(); // gets monitor 
	const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor); // stores it in a pointer
	monitorWidth = mode->width;// stores its width and height for window creation.
	monitorHeight = mode->height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);// sets OpenGl version program will use.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(monitorWidth, monitorHeight, "Plant Simulator V2.0", NULL, NULL);//creates window and sizes it to device monitor size.
	if (window == NULL)// error message 1.
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//allows resizing of the window.
	glfwSetScrollCallback(window, mousescrollcallback); // sets scroll callback for mouse.
	glfwSetMouseButtonCallback(window, mouseclickcallback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//error message 2.
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 1;
	}

	//creat imagui instance.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330"); // Use GLSL version

	//memory of main program.
	point.emplace_back();
	//rendering stuff below.
	Shader shader1("Vertexshader1.txt", "Fragmentshader1.txt");//creates shaders this one will be used for the graphs and other things.
	Shader shadergraph("vertexforback.txt","fragmentforback.txt"); // used only for backround squares and other instanced stuff with offsets that move.

	float GRAPH_VERTICES[24]// defines graph square size.
	{
	0.0, 0.0, 0.0, 0.4, 0.4, 0.4, //format X Y Z R G B
	10.0, 0.0, 0.0, 0.4, 0.4, 0.4,
	10.0, 10.0, 0.0, 0.4, 0.4, 0.4,
	0.0, 10.0, 0.0, 0.4, 0.4, 0.4
	};
	Graph graph1(GRAPH_VERTICES, 4, sizeof(GRAPH_VERTICES));// creates a graph to be used and rendered.
	//creates all the plot lines for each thing graphed.
	plotline Pvl(max_number_of_points, 1.0, 0.0, 0.0);
	plotline Col(max_number_of_points, 0.0, 0.0, 1.0);
	plotline Spl(max_number_of_points, 0.0, 1.0, 0.0);

	Flag F1(glm::vec3{ 0.7,0.0,0.0 });//creates the flags for getting the value at a point.
	Flag F2(glm::vec3{ 0.0,0.7,0.0 });
	Flag F3(glm::vec3{ 0.0,0.0,0.7 });
	Flag F4(glm::vec3{ 0.7,0.7,0.0 });
	Flag F5(glm::vec3{ 0.0,0.7,0.7 });
	Flag F6(glm::vec3{ 0.7,0.0,0.7 });



	int screens = 0;
	//view port stuff for moving around.
	//creates transform matrix
	glm::mat4 transform = glm::mat4(1.0f);//default transform for creating specialized transforms
	glm::mat4 modeltransform = glm::mat4();
	glm::mat4 viewtransform = glm::mat4();
	glm::mat4 projectiontransform = glm::mat4();

	modeltransform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f)); // model tranform aka position in world relitive to 0.0
	glm::mat4 graphtransform = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));//graph transform.
	glm::mat4 testflag = glm::translate(transform, glm::vec3(0.0f, 0.0f, 0.0f));//flag transform.

	//these 2 below are not object based they should stay the same for every object but the camera would get to move the view matix
	viewtransform = glm::translate(transform, glm::vec3(camx, camy, camz)); // view tranform reverced from expected values for directions
	projectiontransform = glm::ortho(-(monitorWidth/2) / zoom, (monitorWidth / 2) / zoom, -(monitorHeight / 2) / zoom, (monitorHeight / 2) / zoom);// width and height of graph and stuff.

	while (!glfwWindowShouldClose(window))
	{

		deltatime = glfwGetTime() - prevtime;
		prevtime = glfwGetTime();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//screen color R, G, B, A
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		processInput(window);// process inputs from keyboard.

		if (!camdetached)
		{
			camx = -(Pvl.point_pos_step * timestep) + 90;
		}
		if (camx > -90)
		{
			camx = -90;
		}
		
		
	
		//apply changes to camera and zoom.
		viewtransform = glm::translate(transform, glm::vec3(camx, camy, camz));
		projectiontransform = glm::ortho(-(monitorWidth / 2) / zoom, (monitorWidth / 2) / zoom, -(monitorHeight / 2) / zoom, (monitorHeight / 2) / zoom);// width and height of graph and stuff.

		//rendering of graph and plots as well as other stuff.
		graphtransform = glm::translate(transform, glm::vec3(-90.0f + (screens * 10.0f), 0.0f, 0.0f));//moves the backround squares to follow the camera sort of.
		screens = -camx / 10;
		graph1.render(shadergraph, Transformations{ graphtransform, viewtransform, projectiontransform });

		//rendering of the flags
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf1, 0.0f, 0.0f));
		F1.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf2, 0.0f, 0.0f));
		F2.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf3, 0.0f, 0.0f));
		F3.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf4, 0.0f, 0.0f));
		F4.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf5, 0.0f, 0.0f));
		F5.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });
		testflag = glm::translate(transform, glm::vec3(absoluteflagposf6, 0.0f, 0.0f));
		F6.render(shader1, Transformations{ testflag,viewtransform,projectiontransform });

		//update process and controller
		Process1.Update(timestep);
		controller1.Update(timestep);

		//exchange values for next frames calculations

		Process1.Co = controller1.Co;
		controller1.Pv = Process1.Pv;

		size_t current_data_index = point.size() + 1;

		//rendering of the plotlines that will be on the graph.
		Spl.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller1.Sp, current_data_index);
		Col.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller1.Co, current_data_index);
		Pvl.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, Process1.Pv, current_data_index);
		
		current_data_index += -1;
		//collect data for use later/retreving.
		point.emplace_back();
		point[current_data_index].PVv = Process1.Pv;
		point[current_data_index].Cov = controller1.Co;
		point[current_data_index].Spv = controller1.Sp;
		/* The code above is not able to be finished untill the controller and process is finsihed. */

		IMGUI_FRAME(current_data_index); // call the frame for the GUI/GUIs.

		//swap buffers and poll IO events (key pressed mouse moved etc)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 1;

}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	monitorWidth = width;
	monitorHeight = height;
}
void processInput(GLFWwindow* window) // handles standard keyboard presses.
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) //prevents the below inputs from regestering if the fields in the gui are being edited.
	{
		return;
	}
	float camspeed = 100.0f * deltatime;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)//beta camera feature for the program
		{
			camy = camy + -camspeed / (zoom * 0.2);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camy = camy + camspeed / (zoom * 0.2);
		}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camx = camx + camspeed / (zoom * 0.2);
		camdetached = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camx = camx + -camspeed / (zoom * 0.2);
		camdetached = true;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
	{
		camdetached = false;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		absoluteflagposf1 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		absoluteflagposf2 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		absoluteflagposf3 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		absoluteflagposf4 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
	{
		absoluteflagposf5 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
	{
		absoluteflagposf6 = Getworldposfrommouse(window, zoom, timestep, cursorposx, cursorposy, camx, monitorWidth, monitorHeight, monitoroffset);
	}
}
void mousescrollcallback(GLFWwindow*, double xoffset, double yoffset)//deals with scrolling so mouse can be used.
{
	zoom += (yoffset * zoom) * 0.2;
	if (zoom < 0.5)
	{
		zoom = 0.5;
	}
}
void mouseclickcallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{

	}
}
void IMGUI_FRAME(int currentpointpos)
{
	int count;


	// 1. Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. Define the UI window
	{
		ImGui::Begin("Flag Points Info"); // Create a window called "Flag Points Info"
		ImGui::Text("This is the value of the most recent value\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f\n", currentpointpos, point[currentpointpos].PVv, point[currentpointpos].Cov, point[currentpointpos].Spv);

		ImGui::Text("This is the value of the point at the flags");

		count = findcount(timestep, currentpointpos, absoluteflagposf1);
		ImGui::Text("F1\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 1", &absoluteflagposf1, timestep, 1, "%0.3f");
		count = findcount(timestep, currentpointpos, absoluteflagposf2);
		ImGui::Text("F2\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 2", &absoluteflagposf2, timestep, 1, "%0.3f");
		count = findcount(timestep, currentpointpos, absoluteflagposf3);
		ImGui::Text("F3\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 3", &absoluteflagposf3, timestep, 1, "%0.3f");
		count = findcount(timestep, currentpointpos, absoluteflagposf4);
		ImGui::Text("F4\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 4", &absoluteflagposf4, timestep, 1, "%0.3f");
		count = findcount(timestep, currentpointpos, absoluteflagposf5);
		ImGui::Text("F5\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 5", &absoluteflagposf5, timestep, 1, "%0.3f");
		count = findcount(timestep, currentpointpos, absoluteflagposf6);
		ImGui::Text("F6\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f", count, point[count].PVv, point[count].Cov, point[count].Spv);

		ImGui::InputDouble("Time (sec) 6", &absoluteflagposf6, timestep, 1, "%0.3f");

		ImGui::End();
	}
	//Define the next UI window
	{
		static float k = ProcessPTR->k, T1 = ProcessPTR->T1, Td = ProcessPTR->Td;
		ImGui::Begin("Transfer Fuction Values"); // Create a window for the process.

		ImGui::Text("K(Gain)  |  T1(Tau one)  |  Td(Deadtime)");
		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("##x1", &k, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			ProcessPTR->k = k;
		}
		ImGui::SetNextItemWidth(90);
		ImGui::SameLine();
		ImGui::InputFloat("##x2", &T1, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			ProcessPTR->T1 = T1;
		}

		ImGui::SetNextItemWidth(100);
		ImGui::SameLine();
		ImGui::InputFloat("##x3", &Td, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			ProcessPTR->Td = Td;
			ProcessPTR->deadtimechanged = true;
		}

		ImGui::End();
	}

	//Define the controller faceplate UI window
	{
		static float kc = controlPTR->Gain, TI = controlPTR->TI, TD = controlPTR->TD;
		ImGui::Begin("Controller Parameters"); // Create a window for the process.

		ImGui::Text("Kc(Gain)  |  TI(Intergral)  |  TD(Derivative)");
		ImGui::SameLine();
		ImGui::Text("Pv |  Co  |  Sp");

		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("##c1", &kc, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->Gain = kc;
		}
		ImGui::SetNextItemWidth(120);
		ImGui::SameLine();
		ImGui::InputFloat("##c2", &TI, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->TI = TI;
		}

		ImGui::SetNextItemWidth(110);
		ImGui::SameLine();
		ImGui::InputFloat("##c3", &TD, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->TD = TD;
		}

		float sp = controlPTR->Sp, co = controlPTR->Co, pv = ProcessPTR->Pv;
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##co", &co, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && !controlPTR->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->Co = co;
		}
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##sp", &sp, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && controlPTR->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->Sp = sp;
		}

		ImGui::Checkbox("Controller Mode | Check = AUTO", &controlPTR->AUTO);
		ImGui::SameLine();
		ImGui::Checkbox("Controller Action | Check = direct acting", &controlPTR->direct_acting);
		ImGui::End();
	}

	// Debug window for testing purposes.
	{
		ImGui::Begin("Debug Window"); // Create a window for the process.

		ImGui::Text("Mo: %0.3f", controlPTR->Mo);
		
		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}