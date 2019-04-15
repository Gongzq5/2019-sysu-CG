#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "HW4.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 viColor;\n"
"\n"
"out vec3 vColor;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"       gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"       vColor = viColor;\n"
"}\n";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 vColor;\n"
"void main()\n"
"{\n"
"       FragColor = vec4(vColor, 0.8);\n"
"}\n";


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	std::vector<float> cubeVertices{
		// coordinate			// color
		 2.0f,  2.0f,  2.0f,    1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,    0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,	0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,	0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f,	1.0f, 0.0f, 0.0f,
		-2.0f,  2.0f, -2.0f,	0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f,	0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,	0.0f, 1.0f, 0.0f
	};
	
	std::vector<unsigned int> cubeIndices{
		0, 1, 2,    1, 2, 3,
		1, 3, 5,    3, 5, 7,
		0, 1, 5,    0, 4, 5,
		4, 5, 7,    4, 6, 7,
		0, 2, 4,    2, 4, 6,
		2, 3, 7,    2, 6, 7
	};

//	std::vector<float> 
	
	unsigned int cubeVAO, cubeVBO, cubeEBO;
	HW4::bind(cubeVertices, cubeVAO, cubeVBO);
	HW4::bindEBO(cubeIndices, cubeVAO, cubeEBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// as we only have a single shader, we could also just activate our shader once beforehand if we want to 
	glUseProgram(shaderProgram);

	enum Homework_number {
		B1, B2, B3, B4, O1
	};
	int HW_choose = B1;
	bool enableDepthTest = true;
	float direct[2] = { 0.0f, 0.0f };

	// retrieve the matrix uniform locations
	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	unsigned int transLoc = glGetUniformLocation(shaderProgram, "trans");

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 trans = glm::mat4(1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		{
			ImGui::Begin("Choose radio to toggle different homework.");
			ImGui::RadioButton("Basic 1", &HW_choose, B1);
			if (HW_choose == B1) {
				ImGui::Checkbox("Enable GL_DEPTH_TEST", &enableDepthTest);
			}
			ImGui::RadioButton("Basic 2", &HW_choose, B2);
			if (HW_choose == B2) {
				ImGui::SliderFloat("Vertical", &direct[0], -1.0f, 1.0f);
				ImGui::SliderFloat("Horizontal", &direct[1], -1.0f, 1.0f);
			}
			ImGui::RadioButton("Basic 3", &HW_choose, B3);

			ImGui::RadioButton("Basic 4", &HW_choose, B4);
			ImGui::RadioButton("Bonus  ", &HW_choose, O1);
			ImGui::End();
		}
		ImGui::Render();

		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		// render
		// ------

		// update shader uniform
		//int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		//glUniform4f(vertexColorLocation, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glEnable(GL_DEPTH_TEST);

		if (HW_choose == B1) {
			if (!enableDepthTest) {
				glDisable(GL_DEPTH_TEST);
			}
			
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);
			
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			// printf("choose B1\n");
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glBindVertexArray(cubeVAO);
			
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		else if (HW_choose == B2) {
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);
			
			model = glm::translate(model, glm::vec3(direct[1], direct[0], 0.0f));

			model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glBindVertexArray(cubeVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		else if (HW_choose == B3) {
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);
			
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 1.0f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		else if (HW_choose == B4) {
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);
			
			model = glm::rotate(model, 1.8f, glm::vec3(1.0f, 0.0f, 1.0f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			// transform ...
			float scales = cos((float)glfwGetTime()) + 1;
			model = glm::scale(model, glm::vec3(scales, scales, scales));
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}
		else {
			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);

			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			// transform ...
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

			model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			view = glm::mat4(1.0f);
			projection = glm::mat4(1.0f);

			model = glm::rotate(model, 0.5f, glm::vec3(1.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
			model = glm::translate(model, glm::vec3(sin((float)glfwGetTime()) * 20, 0.0f, cos((float)glfwGetTime()) * 20));
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
			projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}