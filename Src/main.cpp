#include <Windows.h>
#include <iostream>
#include <chrono>
#include <assert.h>
#include <cstdlib>
#include <ctime>

#include "../Dependencies/include/glfw3.h"

#include "core/Render.h"
#include "core/Model.h"
#include "core/Scence.h"
#include "shader/Shader.h"


int WINDOW_HEIGHT;
int WINDOW_WIDTH;

std::shared_ptr<Camera> mainCamera;
bool mouse_down = false;
float last_x = -1.0f;
float last_y = -1.0f;
float x_offset = 0;
float y_offset = 0;


void mouse_callback(GLFWwindow* window, double x_pos,double y_pos) {
	if (!mouse_down)	 return;

	if (last_x<0 && last_y<0) {
		last_x = x_pos;
		last_y = y_pos;
		return;
	}
	x_offset += (x_pos - last_x)/ WINDOW_WIDTH;
	y_offset += (y_pos - last_y)/ WINDOW_HEIGHT;
	last_x = x_pos;
	last_y = y_pos;

	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mouse_down = true;
			last_x = -1.0;
			last_y = -1.0;
		}
		else if (action == GLFW_RELEASE) {
			mouse_down = false;
			
		}
	}
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
	mainCamera->UpdateCameraFov(y_offset);
}


int main(int argc, char* argv[]) {
	assert(glfwInit() == GLFW_TRUE);
	//set scence
	std::vector<Renderer*> renderPasses;
	unsigned int seed = time(0);
	srand(seed);
	mainCamera=SCENCES[rand() % sizeof(SCENCES) / sizeof(Scence)].BuildScence(renderPasses);
	//mainCamera = SCENCES[0].BuildScence(renderPasses);
	WINDOW_WIDTH = renderPasses.back()->GetWidth();
	WINDOW_HEIGHT = renderPasses.back()->GetHeight();

	

	//init window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Viewer", NULL, NULL);
	assert(window != NULL);
	glfwMakeContextCurrent(window);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	auto last_time = std::chrono::system_clock::now();
	int frame_count = 0;
	//main loop
	while (!glfwWindowShouldClose(window)) {

		//clear
		for (int i = 0; i < renderPasses.size(); i++)
			renderPasses[i]->Clear();

		//render
		for (int i = 0; i < renderPasses.size(); i++) {
			renderPasses[i]->Draw();
		}

		//FPS
		frame_count++;
		auto curr_time = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - last_time);
		if (duration.count() > 1000) {
			std::cout << "FPS:" << frame_count / (double)duration.count()*1000 << std::endl;
			last_time = curr_time;
			frame_count = 0;
		}
		
		glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, renderPasses.back()->GetFrameBuffer().get());
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (x_offset || y_offset) {
			mainCamera->UpdateCameraPos(x_offset, y_offset);
			x_offset = 0;
			y_offset = 0;
		}
	}

	for (int i = 0; i < renderPasses.size(); i++) {
		delete renderPasses[i];
	}

	glfwTerminate();
	return 0;
}



