#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Bresenham.h"
#include "Bezier.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void drawControllerPoints(GLuint VAO, GLuint VBO, std::vector<Bezier::Point> controllerPoints);
void cursor_position_callback(GLFWwindow* window, double x, double y);

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

double mouse_x, mouse_y;
std::vector<Bezier::Point> controllerPoints;

bool show_assistant = true;
float speed = 0.05f;
float lastTab = -1000.0f;

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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

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
		1.0f, -1.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 
		-1.0f, -1.0f, 0.0f
	};
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Bresenham algorithm
	float triVertices[6] = {-0.9f, 0.2f, 0.9f, 0.7f, 0.1f, -0.5f};
	
	//auto triPoints = Bresenham::genTriPoints(Bresenham::Point(triVertices[0], triVertices[1]),
	//	Bresenham::Point(triVertices[2], triVertices[3]),
	//	Bresenham::Point(triVertices[4], triVertices[5]));
	auto triPoints = Bresenham::genTriPoints(Bresenham::Point(0.9f, 0.9f),
			Bresenham::Point(-0.5f, 0.2f),
			Bresenham::Point(-0.08f, 1.0f));

	unsigned int HW3B1_VAO, HW3B1_VBO;
	Bresenham::pointsBindVAO(HW3B1_VAO, HW3B1_VBO, triPoints);

	// as we only have a single shader, we could also just activate our shader once beforehand if we want to 
	glUseProgram(shaderProgram);

	
	//controllerPoints.push_back(Bezier::Point(-1.0f, -0.9f));
	//controllerPoints.push_back(Bezier::Point(0.9f, 0.9f));
	
	controllerPoints.push_back(Bezier::Point(-0.9f, -0.5f));
	controllerPoints.push_back(Bezier::Point(-0.9f, 0.2f));
	controllerPoints.push_back(Bezier::Point(0.7f, 0.8f));
	
	float tLimit = 0.0f, lastTime = 0.0f;

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
		ImGui::Text("TAB to switch whether to show the assistant line, up and down to adjust speed.");
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

		if (!show_assistant) {
			tLimit = 1.0f;
		} else if ((float)glfwGetTime() - lastTime > 0.1f) {
			lastTime = (float)glfwGetTime();
			tLimit += speed;
			if (tLimit >= 1.0f) {
				tLimit = 0.0f;
			}
		}

		std::vector<float> triPoints = Bezier::genBezierCurvePoints(controllerPoints, tLimit);

		if (show_assistant) {
			std::vector<float> assistantLine = Bezier::genAssistantLinePoints(controllerPoints, tLimit);
			triPoints.insert(triPoints.end(), assistantLine.begin(), assistantLine.end());
		}

		glBindVertexArray(HW3B1_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, HW3B1_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * triPoints.size(), triPoints.data(), GL_DYNAMIC_DRAW);
		glPointSize(1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_POINTS, 0, triPoints.size() / 3);

		drawControllerPoints(VAO, VBO, controllerPoints);
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------ 
	glDeleteVertexArrays(1, &HW3B1_VAO);
	glDeleteBuffers(1, &HW3B1_VBO);
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
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		speed += 0.001f;
		if (speed > 0.5f) speed = 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		speed -= 0.001f;
		if (speed < 0.01f) speed = 0.01f;
	}
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		printf("%f\n", glfwGetTime());
		if (glfwGetTime() - lastTab > 0.3f) {
			show_assistant = !show_assistant;
			lastTab = glfwGetTime();
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void deleteControllerPoint(float mouse_x, float mouse_y) {
	float gap = 0.02f;
	for (auto it = controllerPoints.begin(); it != controllerPoints.end(); it++) {
		if (abs(it->x - mouse_x) < gap && abs(it->y - mouse_y) < gap) {
			it = controllerPoints.erase(it);
			break;
		}
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			controllerPoints.insert(controllerPoints.end(), Bezier::Point(mouse_x, mouse_y));
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "Mosue middle button clicked!" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "Mosue right button clicked!" << std::endl;
			deleteControllerPoint(mouse_x, mouse_y);
			break;
		default:
			return;
		}
	}
	return;
}

void drawControllerPoints(GLuint VAO, GLuint VBO, std::vector<Bezier::Point> controllerPoints)
{
	std::vector<float> controllerPointsVecFloat3 = Bezier::pointsToFloat3(controllerPoints);
	//std::cout << controllerPointsVecFloat3[0] << " " << controllerPointsVecFloat3[1] << " " << controllerPointsVecFloat3[2] << std::endl;
	//std::cout << controllerPointsVecFloat3[3] << " " << controllerPointsVecFloat3[4] << " " << controllerPointsVecFloat3[5] << std::endl;

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * controllerPointsVecFloat3.size(), controllerPointsVecFloat3.data(), GL_STATIC_DRAW);
	glPointSize(10);
	glDrawArrays(GL_POINTS, 0, controllerPointsVecFloat3.size() / 3);
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	mouse_x = float((x - SCR_WIDTH / 2) / SCR_WIDTH) * 2;
	mouse_y = float(0 - (y - SCR_HEIGHT / 2) / SCR_HEIGHT) * 2;
}