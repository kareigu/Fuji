#include "window.h"

#include <fmt/core.h>


#include <vector>

namespace core {

	bool Window::shouldRun() {
		return m_window ? !glfwWindowShouldClose(m_window) : false;
	}

	void Window::poll() {
		glfwPollEvents();
	}

	void Window::close() {
		vkDestroyInstance(m_instance, nullptr);
		glfwDestroyWindow(m_window);
		glfwTerminate();
		fmt::print("Terminating\n");
	}

	int Window::init() {
		if (!glfwInit())
			return -1;
		fmt::print("Init\n");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
		if (!m_window) {
			glfwTerminate();
			fmt::print("Couldn't create Window\n");
			return EXIT_FAILURE;
		}
		fmt::print("Created Window({:p})\n", (void*)m_window);

		uint32_t glfwExtensionCount = 0;
		const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		fmt::print("{} extensions supported\n", glfwExtensionCount);

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwRequiredExtensions;
		createInfo.enabledLayerCount = 0;


		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		fmt::print("available extensions:\n");

		for (const auto& extension : extensions) {
			fmt::print("\t {}\n", extension.extensionName);
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			fmt::print("Couldn't create VkInstance\n");
			return EXIT_FAILURE;
		}


		if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
			fmt::print("Couldn't create WindowSurface\n");
			return EXIT_FAILURE;
		}
		fmt::print("Created VkSurface({:p}) for Window({:p})\n", (void*)&m_surface, (void*)m_window);

		return EXIT_SUCCESS;
	}

	Window::Window(const int width, const int height, std::string window_title) 
		: m_width(width), m_height(height), m_title(window_title) {}

	Window::~Window() {};
}