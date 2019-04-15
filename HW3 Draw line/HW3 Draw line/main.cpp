#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Bresenham.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
"}\n\0";


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

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------	
	float vertices[] = {
		1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// ÑÕÉ«ÊôÐÔ
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Bresenham algorithm
	int triVertices[6] = {-90, 20, 90, 70, 10, -50};
	
	//auto triPoints = Bresenham::genTriPoints(Bresenham::Point(triVertices[0], triVertices[1]),
	//	Bresenham::Point(triVertices[2], triVertices[3]),
	//	Bresenham::Point(triVertices[4], triVertices[5]));
	auto triPoints = Bresenham::genTriPoints(Bresenham::Point(90, 90),
			Bresenham::Point(-50, 20),
			Bresenham::Point(-8, 100));

	unsigned int HW3B1_VAO, HW3B1_VBO;
	Bresenham::pointsBindVAO(HW3B1_VAO, HW3B1_VBO, triPoints);

	int radius = 50;
	auto circlePoints = Bresenham::genCirclePositions(Bresenham::Point(0, 0), radius);
	unsigned int HW3B2_VAO, HW3B2_VBO;
	Bresenham::pointsBindVAO(HW3B2_VAO, HW3B2_VBO, circlePoints);

	auto filledTriPoints = Bresenham::genFilledTriPoints(Bresenham::Point(triVertices[0], triVertices[1]),
		Bresenham::Point(triVertices[2], triVertices[3]),
		Bresenham::Point(triVertices[4], triVertices[5]));
	unsigned int HW3N1_VAO, HW3N1_VBO;
	Bresenham::pointsBindVAO(HW3N1_VAO, HW3N1_VBO, filledTriPoints);

	// as we only have a single shader, we could also just activate our shader once beforehand if we want to 
	glUseProgram(shaderProgram);

	enum Homework_number {
		HW3_Basic1,
		HW3_Basic2,
		HW3_Bonus1
	};
	int HW_choose = HW3_Basic1;
	
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
			ImGui::RadioButton("HW3 Basic1", &HW_choose, HW3_Basic1);
			ImGui::RadioButton("HW3 Basic2", &HW_choose, HW3_Basic2);
			if (HW_choose == HW3_Basic2) {
				ImGui::Text("Radius");
				ImGui::SliderInt("", &radius, 0, 100);
			} else if (HW_choose == HW3_Bonus1 || HW_choose == HW3_Basic1) {
				ImGui::Text("Choose three vertices.");
				ImGui::SliderInt("x1", &triVertices[0], -100, 100);
				ImGui::SliderInt("y1", &triVertices[1], -100, 100);
				ImGui::SliderInt("x2", &triVertices[2], -100, 100);
				ImGui::SliderInt("y2", &triVertices[3], -100, 100);
				ImGui::SliderInt("x3", &triVertices[4], -100, 100);
				ImGui::SliderInt("y3", &triVertices[5], -100, 100);
			}
			ImGui::RadioButton("HW3 Bonus1", &HW_choose, HW3_Bonus1);
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
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		glUniform4f(vertexColorLocation, clear_color.x, clear_color.y, clear_color.z, clear_color.w);

		if (HW_choose == HW3_Basic1) {
			triPoints = Bresenham::genTriPoints(Bresenham::Point(triVertices[0], triVertices[1]),
				Bresenham::Point(triVertices[2], triVertices[3]),
				Bresenham::Point(triVertices[4], triVertices[5]));

			glBindVertexArray(HW3B1_VAO);
			glBindBuffer(GL_ARRAY_BUFFER, HW3B1_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * triPoints.size(), triPoints.data(), GL_DYNAMIC_DRAW);
			glPointSize(3);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_POINTS, 0, triPoints.size() / 3);
		} else if (HW_choose == HW3_Basic2) {

			circlePoints = Bresenham::genCirclePositions(Bresenham::Point(0, 0), radius);
			
			glBindVertexArray(HW3B2_VAO);
			glBindBuffer(GL_ARRAY_BUFFER, HW3B2_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * circlePoints.size(), circlePoints.data(), GL_DYNAMIC_DRAW);
			
			glPointSize(3);

			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_POINTS, 0, circlePoints.size() / 3);
		} else if (HW_choose == HW3_Bonus1) {
			
			filledTriPoints = Bresenham::genFilledTriPoints(Bresenham::Point(triVertices[0], triVertices[1]),
				Bresenham::Point(triVertices[2], triVertices[3]),
				Bresenham::Point(triVertices[4], triVertices[5]));
			
			glBindVertexArray(HW3N1_VAO);
			glBindBuffer(GL_ARRAY_BUFFER, HW3N1_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * filledTriPoints.size(), filledTriPoints.data(), GL_DYNAMIC_DRAW);
			
			glPointSize(3);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_POINTS, 0, filledTriPoints.size() / 3);
		}
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//delete []HW3B1_vertices;
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	//glDeleteVertexArrays(1, &HW2B3_VAO);
	//glDeleteBuffers(1, &HW2B3_VBO); 
	glDeleteVertexArrays(1, &HW3B1_VAO);
	glDeleteBuffers(1, &HW3B1_VBO);
	glDeleteVertexArrays(1, &HW3N1_VAO);
	glDeleteBuffers(1, &HW3N1_VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
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