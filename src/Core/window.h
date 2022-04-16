#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>


namespace core {
	class Window {
	public:
		Window(const int width, const int height, std::string window_title);
		~Window();

		int init();

		bool shouldRun();

		void close();

		void poll();

	private:
		const int m_width = 800;
		const int m_height = 600;
		std::string m_title{};

		GLFWwindow* m_window = nullptr;
		VkInstance m_instance{};
		VkSurfaceKHR m_surface{};
	};
}


#endif
