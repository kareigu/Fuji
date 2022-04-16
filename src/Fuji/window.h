#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <optional>

namespace fuji {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics;

		bool ready() { return graphics.has_value(); }
	};

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

		int pickPhysicalDevice();
		int createLogicalDevice();

		GLFWwindow* m_window = nullptr;
		VkInstance m_instance{};
		VkSurfaceKHR m_surface{};

		VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;

		QueueFamilyIndices m_queue_families{};
		VkQueue m_graphics_queue = VK_NULL_HANDLE;
	};
}


#endif
