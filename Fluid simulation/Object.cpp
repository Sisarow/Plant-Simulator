#include "Object.h"

//graph code
Graph::Graph(float Veritces[], int num_of_points, int size) //builds the pre stuff for the graph.
{
	glGenVertexArrays(1, &VAO);//pv vao
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);											//this section binds all the vbos to the standard VAO for redering the graph values.
	glBufferData(GL_ARRAY_BUFFER, size, Veritces, GL_DYNAMIC_DRAW);// states how many vertices are in object/ how much information/space it will take.
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	int coloum = 0, row = 0;
	for (int i = 0; i < 220; i++)
	{
		if (i % 10 == 0 && i != 0)
		{
			coloum++;
			row = 0;
		}
		graphboxpos[i] = (glm::vec3(-10.0f + (coloum * 10.0f), 0.0f + (row * 10.0f), 0.0f));
		row++;
	}

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 220, &graphboxpos, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1);
	num_of_points_s = num_of_points;
}

void Graph::render(const Shader& shader,const Transformations& Transform) // NOTE PASS BY REFERENCE TO AVOID DECONSTUCTING THE SHADER THROWING AN EXCEPTION.
{
	shader.use();
	shader.setmat4("model", Transform.modeltransform);
	shader.setmat4("view", Transform.viewtransform);
	shader.setmat4("projection", Transform.projectiontransform);
	glLineWidth(1.0f);
	glBindVertexArray(VAO);
	glDrawArraysInstanced(GL_LINE_LOOP, 0, num_of_points_s, 220);
}

//plotline code
plotline::plotline(unsigned long long int max_num_of_points, glm::vec3 color_input)
{
	color = color_input;
	Maximum_graphed_floats = max_num_of_points * 6.0f;
	glGenVertexArrays(1, &VAO);//pv vao
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);											//this section binds all the vbos to the standard VAO for redering the graph values.
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Maximum_graphed_floats, nullptr, GL_DYNAMIC_DRAW);// states how many vertices are in object/ how much information/space it will take.
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}
plotline::plotline(unsigned long long int max_num_of_points, float color_red, float color_green, float color_blue)
{
	color = glm::vec3{color_red, color_green, color_blue};
	Maximum_graphed_floats = max_num_of_points * 6.0f;
	glGenVertexArrays(1, &VAO);//pv vao
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);											//this section binds all the vbos to the standard VAO for redering the graph values.
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Maximum_graphed_floats, nullptr, GL_DYNAMIC_DRAW);// states how many vertices are in object/ how much information/space it will take.
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void plotline::render(const Shader& shader, const Transformations& Transform, float timestep, float input_value, float index_point_value)
{
	plotline::Update(timestep, input_value, index_point_value);
	shader.use();
	glLineWidth(5.0f);
	shader.setmat4("model", Transform.modeltransform);
	shader.setmat4("view", Transform.viewtransform);
	shader.setmat4("projection", Transform.projectiontransform);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, 0 + point_pos_step);
}

void plotline::Update(float timestep, float input_value, float index_point_value)
{
	point_pos_step++; //increments point step for point id.
	Total_time = index_point_value * timestep; // adds timestep to total elapsed time.
	float newpvvertice[6]
	{
	Total_time,input_value,0.0f,color.x,color.y,color.z//updates graph point.
	};
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0 + (24 * point_pos_step), sizeof(float) * 6, newpvvertice); // adds point to renderer.
}

//Flag code below
Flag::Flag(glm::vec3 color)
{
	float FLAG_VERTICES[24]
	{
		0.0,0.0,-0.1,color.x,color.y,color.z,
		0.0,110.0,-0.1,color.x,color.y,color.z,
		-5.0,105.0,-0.1,color.x,color.y,color.z,
		0.0,100.0,-0.1,color.x,color.y,color.z,
	};

	glGenVertexArrays(1, &VAO);//pv vao
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);											//this section binds all the vbos to the standard VAO for redering the graph values.
	glBufferData(GL_ARRAY_BUFFER, sizeof(FLAG_VERTICES), FLAG_VERTICES, GL_DYNAMIC_DRAW);// states how many vertices are in object/ how much information/space it will take.
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	num_of_points_s = 4;
}
void Flag::render(const Shader& shader, const Transformations& Transform)
{
	shader.use();
	glLineWidth(1.5f);
	shader.setmat4("model", Transform.modeltransform);
	shader.setmat4("view", Transform.viewtransform);
	shader.setmat4("projection", Transform.projectiontransform);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_STRIP, 0, num_of_points_s);
}