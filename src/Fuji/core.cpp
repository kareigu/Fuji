#include "core.h"
#include <fmt/core.h>

#include <vector>

#include "window.h"

namespace fuji {
	int run() {
		Window window(1024, 768, "Fuji Engine");

		if (window.init())
			return EXIT_FAILURE;

		while (window.shouldRun()) {
			//glClear(GL_COLOR_BUFFER_BIT);

			//glfwSwapBuffers(window);

			window.poll();
			window.draw();
		}

		window.close();
		return EXIT_SUCCESS;
	}
}
