
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
#include "SimView.h" //used to manage the child window for the plane simuator.
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

void framebuffer_size_callback(GLFWwindow*, int, int);
void mousescrollcallback(GLFWwindow*, double, double);
void processInput(GLFWwindow*, GLFWwindow*);
void mouseclickcallback(GLFWwindow*, int, int, int);
void Test_Master(int Point_pos);

void IMGUI_FRAME(int);
int monitorWidth, monitorHeight, monitoroffset = 0;
bool camdetached = false;

bool testingmode = false;

const unsigned int max_number_of_points = 200000; //defaut is 300000 points which will make sure memory is not run out during operation.
float timestep = 0.010; // desired time step for desred accuacy of simulation (also can affect simulation speed!). 
float camx = -90, camy = -50, camz = 0, zoom = 10; // for camera stuffs
float deltatime, prevtime; // for finding time between frames.
Processsim* ProcessPTR;
Controller* controlPTR;
Controller* controlPTR2;
Controller* controlPTR3;
std::vector<Point_Data> point;

double cursorposx, cursorposy;

double absoluteflagposf1 = 0;// all input flags which can be controlled with numbers 1-6 on keyboard respectively.
double absoluteflagposf2 = 0;
double absoluteflagposf3 = 0;
double absoluteflagposf4 = 0;
double absoluteflagposf5 = 0;
double absoluteflagposf6 = 0;


GLFWwindow* simview_window = NULL;
bool childshouldclose = true;
char type_of_test = NULL; //for test type
bool run = true;

int main()
{
	std::thread second_window;

	//Setup of Process simulator and Controller.
	Processsim Process1;
	ProcessPTR = &Process1;

	//setup of controller.
	Controller controller1, controller2, controller3;
	controller1.Sp = 50;
	controller1.Co = 50.49;
	controller1.Gain = 0.010;
	controller1.TI = 0.03;
	controller1.oldintergrated = 50.488930;
	controller1.smart = false;


	controller2.Gain = 0.1; //carefull of gain since feedback noise exsists.
	controller2.TI = 0.05; //need fast tau  for good control
	controller2.TD = 0.0;
	controller2.Sp = 50.49;
	controller2.Co = 49.85;
	controller2.oldintergrated = 49.852482;
	controller2.direct_acting = true;

	controller3.Sp = 45;
	controller3.Co = 19.90;
	controller3.oldintergrated = 19.903376;
	controller3.TI = 0.2;
	controller3.Gain = 6;

	controlPTR = &controller1;
	controlPTR2 = &controller2;
	controlPTR3 = &controller3;

	if (Process1.process == '3')
	{
		bool incorrect = true;
		do
		{
			printf("Enable Testing Mode? (Y/N)\n");
			char input;
			std::cin >> input;
			if (input == 'Y' || input == 'y')
			{
				printf("Enabled!\n");
				testingmode = true;
				incorrect = false;
			}
			else if (input == 'N' || input == 'n')
			{
				printf("Disabled!\n");
				incorrect = false;
			}
			else
			{
				printf("Incorrect input\nTry again.");
			}

		} while (incorrect);

		if (testingmode)
		{
			printf("Please select the type of Test:\n-------------------------------\n");
			printf("1 = (Regular cascade test)\n");
			printf("2 = (smart controller cascade test type B (minimal learning))\n");
			printf("3 = (smart controller cascade test type A (moderate learning))\n");
			printf("4 = (smart controller cascade test type SS (higher learning))\n");
			printf("5 = (smart controller cascade test type L (defult learning == no runtime learning but still smart. Pre-trained/tuned))\n");
			std::cin >> type_of_test;
			switch (type_of_test)
			{
			case '1':
				controller1.Gain = 1.3;
				controller1.TI = 7.0;
				controller1.oldintergrated = 50.488930;
				controller1.smart = false;
				break;
			case '2':
				controller1.Gain = 0.010;
				controller1.TI = 0.03;
				controller1.oldintergrated = 50.488930;
				controller1.smart = true;
				break;
			case '3':
				controller1.Gain = 0.010;
				controller1.TI = 0.02;
				controller1.oldintergrated = 50.488930;
				controller1.smart = true;
				break;
			case '4':
				controller1.Gain = 0.010;
				controller1.TI = 0.02;
				controller1.oldintergrated = 50.488930;
				controller1.smart = true;
				break;
			case '5':
				controller1.Gain = 0.010;
				controller1.TI = 0.03;
				controller1.oldintergrated = 50.488930;
				controller1.smart = true;
				controller1.enable_RLS = false;
				break;
			default:
				printf("Please enter another number.\n");
				break;
			}

		}
	}

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
	if (Process1.process == '3')
	{
		simview_window = glfwCreateWindow(monitorWidth, monitorHeight, "SimView", NULL, window);//creates window and sizes it to device monitor size.
		childshouldclose = false;
		glfwHideWindow(simview_window);
		second_window = std::thread(Child_window);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//allows resizing of the window.
	glfwSetScrollCallback(window, mousescrollcallback); // sets scroll callback for mouse.
	glfwSetMouseButtonCallback(window, mouseclickcallback);
	glfwSwapInterval(0);
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



	plotline Pv2(max_number_of_points, 1.0, 0.6549, 0.0);
	plotline Co2(max_number_of_points, (199.0 / 255.0), (44.0/255.0), (161.0/255.0));
	//plotline Sp2(max_number_of_points, 0.6, 0.9, 0.2);
	//plotline test1(max_number_of_points, 1.0, 1.0, 0);
	//plotline test2(max_number_of_points, 0.0,0.0, 0.0);
	//plotline test3(max_number_of_points, 1.0, 0.0, 1.0);

	while (!glfwWindowShouldClose(window))
	{

		deltatime = glfwGetTime() - prevtime;
		prevtime = glfwGetTime();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//screen color R, G, B, A
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		processInput(window, simview_window);// process inputs from keyboard.

		if (!camdetached)
		{
			camx = -(point.size() * timestep) + 90;
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

		//exchange values for next frames calculations

		Process1.Co = controller2.Co;
		controller2.Pv = (((Process1.state.theta * 180 / 3.141592653589) - -20) / 40) * 100;
		controller2.Sp = controller1.Co;
		controller1.Pv = (-Process1.state.z / 20000) * 100;

		controller3.Pv = (Process1.state.u / 200) * 100;
		Process1.throttle = (controller3.Co / 100) * 5;

		//update process and controller
		Process1.Update(timestep);
		controller1.Update(timestep);
		controller2.Update(timestep);
		controller3.Update(timestep);

		size_t current_data_index = point.size() + 1;

		//rendering of the plotlines that will be on the graph.
		Spl.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller1.Sp, current_data_index);
		Col.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller1.Co, current_data_index);
		Pvl.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, Process1.Pv, current_data_index);
		//Sp2.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller2.Sp, current_data_index);
		Co2.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller2.Co, current_data_index);
		Pv2.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, (((Process1.state.theta * 180 / 3.141592653589) - -20) / 40) * 100, current_data_index);

		//test1.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller3.Pv, current_data_index);
		//test2.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller3.Co, current_data_index);
		//test3.render(shader1, Transformations{ modeltransform,viewtransform,projectiontransform }, timestep, controller3.Sp, current_data_index);

		current_data_index += -1;
		//collect data for use later/retreving.
		point.emplace_back();
		point[current_data_index].PVv = Process1.Pv;
		point[current_data_index].Cov = controller1.Co;
		point[current_data_index].Spv = controller1.Sp;
		point[current_data_index].a1 = controller1.theta[0];
		point[current_data_index].b1 = controller1.theta[1];
		point[current_data_index].c1 = controller1.theta[2];
		point[current_data_index].error = controller1.errorsaved;
		point[current_data_index].prediction = controller1.predictionsaved;

		IMGUI_FRAME(current_data_index); // call the frame for the GUI/GUIs.

		//swap buffers and poll IO events (key pressed mouse moved etc)
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (testingmode)
		{
			Test_Master(current_data_index);
		}
	}
	if (second_window.joinable())
	{
		second_window.join();
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
void processInput(GLFWwindow* window, GLFWwindow* sim_window) // handles standard keyboard presses.
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) //prevents the below inputs from regestering if the fields in the gui are being edited.
	{
		return;
	}
	float camspeed = 100.0f * deltatime;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		if (sim_window != NULL)
		{
			childshouldclose = true;
		}
	}
	if (sim_window != NULL)
	{
		if (glfwGetKey(sim_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(sim_window, true);
			glfwHideWindow(sim_window);
		}
	}
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
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && simview_window != NULL)
	{

		glfwSetWindowShouldClose(sim_window, false);
		glfwShowWindow(sim_window);
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
	{
		run = true;
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
		ImGui::Text("F1\nCount: %i, Pv: %0.2f, Co: %0.2f, Sp: %0.2f    %0.4f %0.4f %0.4f   %0.4f %0.4f", count, point[count].PVv, point[count].Cov, point[count].Spv, point[count].a1, point[count].b1, point[count].c1, point[count].error, point[count].prediction);

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
		ImGui::Text("	Co  |  Sp");

		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("##c1", &kc, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->Gain = kc;
		}
		ImGui::SetNextItemWidth(120);
		ImGui::SameLine();
		ImGui::InputFloat("##c2", &TI, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR->TI = TI;
		}

		ImGui::SetNextItemWidth(110);
		ImGui::SameLine();
		ImGui::InputFloat("##c3", &TD, 0, 0, "%0.3f");
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

		//2nd controller
		static float kc2 = controlPTR2->Gain, TI2 = controlPTR2->TI, TD2 = controlPTR2->TD;
		ImGui::Text("Kc(Gain)  |  TI(Intergral)  |  TD(Derivative)");
		ImGui::SameLine();
		ImGui::Text("	Co  |  Sp");

		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("##c12", &kc2, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR2->Gain = kc2;
		}
		ImGui::SetNextItemWidth(120);
		ImGui::SameLine();
		ImGui::InputFloat("##c22", &TI2, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR2->TI = TI2;
		}

		ImGui::SetNextItemWidth(110);
		ImGui::SameLine();
		ImGui::InputFloat("##c32", &TD2, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR2->TD = TD2;
		}

		float sp2 = controlPTR2->Sp, co2 = controlPTR2->Co, pv2 = controlPTR2->Pv;
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##co2", &co2, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && !controlPTR2->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR2->Co = co2;
		}
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##sp2", &sp2, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && controlPTR2->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR2->Sp = sp2;
		}

		ImGui::Checkbox("Controller Mode | Check = AUTO ", &controlPTR2->AUTO);
		ImGui::SameLine();
		ImGui::Checkbox("Controller Action | Check = direct acting ", &controlPTR2->direct_acting);


		//3rd controller
		static float kc3 = controlPTR3->Gain, TI3 = controlPTR3->TI, TD3 = controlPTR3->TD;
		ImGui::Text("Kc(Gain)  |  TI(Intergral)  |  TD(Derivative)");
		ImGui::SameLine();
		ImGui::Text("	Co  |  Sp");

		ImGui::SetNextItemWidth(70);
		ImGui::InputFloat("##c13", &kc3, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR3->Gain = kc3;
		}
		ImGui::SetNextItemWidth(120);
		ImGui::SameLine();
		ImGui::InputFloat("##c23", &TI3, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR3->TI = TI3;
		}

		ImGui::SetNextItemWidth(110);
		ImGui::SameLine();
		ImGui::InputFloat("##c33", &TD3, 0, 0, "%0.3f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR3->TD = TD3;
		}

		float sp3 = controlPTR3->Sp, co3 = controlPTR3->Co, pv3 = controlPTR3->Pv;
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##co3", &co3, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && !controlPTR3->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR3->Co = co3;
		}
		ImGui::SetNextItemWidth(60);
		ImGui::SameLine();
		ImGui::InputFloat("##sp3", &sp3, 0, 0, "%0.2f");
		if (ImGui::IsItemDeactivatedAfterEdit() && controlPTR3->AUTO) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			controlPTR3->Sp = sp3;
		}

		ImGui::Checkbox("Controller Mode | Check = AUTO  ", &controlPTR3->AUTO);
		ImGui::SameLine();
		ImGui::Checkbox("Controller Action | Check = direct acting  ", &controlPTR3->direct_acting);

		ImGui::End();


	}

	// Debug window for testing purposes.
	{
		ImGui::Begin("Debug Window"); // Create a window for the process.

		ImGui::Text("slope of Co: %0.9f", controlPTR->saved_slope);

		ImGui::Text("plant parameters: %0.3f  %0.3f %0.3f", controlPTR->Sa1, controlPTR->Sb1, controlPTR->Sc1);
		ImGui::Text("controller guess error: %0.3f", controlPTR->errorsaved);
		ImGui::Text("Speedx M/s: %0.3f", ProcessPTR->state.u);
		ImGui::Text("Speedy M/s: %0.3f", ProcessPTR->state.v);
		ImGui::Text("Speedz M/s: %0.3f", ProcessPTR->state.w);
		ImGui::Text("AoA: % 0.3f",(atan2(ProcessPTR->state.w, ProcessPTR->state.u)*180)/3.141592653589);
		ImGui::Text("pitch: %0.3f", ProcessPTR->state.theta*180/3.141592653589);
		ImGui::Text("timetorun: %0.5f", controlPTR->timetorun *1000 * 1000);


		double throt = ProcessPTR->throttle;
		ImGui::InputDouble("Throttle", &throt, 0, 0, "%0.1f");
		if (ImGui::IsItemDeactivatedAfterEdit()) // used to only update value when enter key is pressed or when user clicks away from screen.
		{
			ProcessPTR->throttle = throt;
			printf("plane values: %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf %0.2lf \n", ProcessPTR->state.u, ProcessPTR->state.v, ProcessPTR->state.w, ProcessPTR->state.p, ProcessPTR->state.q, ProcessPTR->state.r, ProcessPTR->state.phi, ProcessPTR->state.theta, ProcessPTR->state.psi, ProcessPTR->state.x, ProcessPTR->state.y, ProcessPTR->state.z);
			printf("controller2 values: %f %f \n", controlPTR2->oldintergrated, controlPTR2->prev_error);
			printf("controller1 values: %f %f\n", controlPTR->oldintergrated, controlPTR->prev_error);
			printf("controller3 values: %f %f\n", controlPTR3->oldintergrated, controlPTR3->prev_error);
		}

		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
bool used1 = false,used2 =false, used3 = false, used4 = false, used5 = false, used6 = false, used7 = false, used8 = false;
void Test_Master(int point_pos)
{
	if (point_pos  % 10000 == 0 && !used1 && run)
	{
		controlPTR->Sp = 60;
		used1 = true;
		run = false;
	}
	if (point_pos % 10000 == 0 && !used2 && run)
	{
		controlPTR->Sp = 40;
		used2 = true;
		run = false;
	}
	if (point_pos % 10000 == 0 && !used5 && run)
	{
		ProcessPTR->rho -= 0.3;
		run = false;
		used5 = true;
	}
	if (point_pos % 10000 == 0 && !used6 && run)
	{
		ProcessPTR->wind_bias = 0.05;
		run = false;
		used6 = true;
	}
	if (point_pos % 10000 == 0 && (type_of_test == '3' || type_of_test == '4') && !used3 && run)
	{
		controlPTR->Sp = 50;
		used3 = true;
		run = false;
	}
	if (point_pos % 10000 == 0 && type_of_test == '4' && !used4 && run)
	{
		controlPTR->Sp = 65;
		run = false;
		used4 = true;
	}
	if (point_pos % 10000 == 0 && !used7 && run)
	{
		controlPTR->Sp = 55;
		run = false;
		used7 = true;
	}
	if (point_pos % 10000 == 0 && !used8 && run)
	{
		controlPTR->Sp = 45;
		used8 = true;
		run = false;
	}
}