#include "core.h"
#include <fmt/core.h>
#include <GLFW/glfw3.h>

namespace core {
	int run() {
		GLFWwindow* window;

		if (!glfwInit())
			return -1;
		fmt::print("Init\n");

		window = glfwCreateWindow(800, 600, "Engine", NULL, NULL);
		if (!window) {
			glfwTerminate();
			fmt::print("Couldn't create window\n");
			return -1;
		}
		fmt::print("Created window {:p}\n", static_cast<void*>(window));

		glfwMakeContextCurrent(window);
		fmt::print("Set rendering context {:p}\n", static_cast<void*>(window));

		while (!glfwWindowShouldClose(window)) {
			glClear(GL_COLOR_BUFFER_BIT);

			glfwSwapBuffers(window);

			glfwPollEvents();
		}

		glfwTerminate();
		fmt::print("Terminating\n");
		return 0;
	}
}
