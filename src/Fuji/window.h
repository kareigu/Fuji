#ifndef _CORE_WINDOW_H_
#define _CORE_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <optional>
#include <vector>

namespace fuji {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		bool ready() { return graphics.has_value() && present.has_value(); }
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;

		bool ready() { return !formats.empty() && !present_modes.empty(); }
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
		SwapChainSupportDetails m_sc_support_details{};
		VkQueue m_graphics_queue = VK_NULL_HANDLE;
		VkQueue m_present_queue = VK_NULL_HANDLE;
	};
}


#endif
