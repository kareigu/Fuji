#include "window.h"

#include <fmt/core.h>


#include <vector>
#include <optional>




namespace fuji {

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families) {
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				if (indices.ready())
					break;
				indices.graphics = i;
			}

			i++;
		}

		return indices;
	}

	static bool deviceSuitable(VkPhysicalDevice device, QueueFamilyIndices queue_family_indices) {
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceProperties(device, &device_properties);
		vkGetPhysicalDeviceFeatures(device, &device_features);


		return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader && queue_family_indices.ready();
	}


	bool Window::shouldRun() {
		return m_window ? !glfwWindowShouldClose(m_window) : false;
	}

	void Window::poll() {
		glfwPollEvents();
	}

	void Window::close() {
		vkDestroyDevice(m_device, nullptr);
		fmt::print("Destroyed VkDevice\n");
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		fmt::print("Destroyed VkSurfaceKHR\n");
		vkDestroyInstance(m_instance, nullptr);
		fmt::print("Destroyed VkInstance\n");
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
		fmt::print("{} GLFW extensions supported\n", glfwExtensionCount);

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


		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> extensions(extension_count);

		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

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

		if (pickPhysicalDevice()) {
			fmt::print("Failed getting VkPhysicalDevice\n");
			return EXIT_FAILURE;
		}
		
		fmt::print("Picked VkPhysicalDevice({:p})\n", (void*)&m_physical_device);

		if (createLogicalDevice()) {
			fmt::print("Failed creating VkDevice\n");
		}

		fmt::print("Created VkDevice({:p})\n", (void*)&m_device);

		vkGetDeviceQueue(m_device, m_queue_families.graphics.value(), 0, &m_graphics_queue);
		fmt::print("Got m_graphics_queue handle ({:p})\n", (void*)&m_graphics_queue);

		return EXIT_SUCCESS;
	}

	Window::Window(const int width, const int height, std::string window_title) 
		: m_width(width), m_height(height), m_title(window_title) {}

	Window::~Window() {};

	int Window::pickPhysicalDevice() {
		uint32_t device_count;
		vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

		std::vector<VkPhysicalDevice> physical_devices(device_count);
		vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());


		for (const auto& device : physical_devices) {
			m_queue_families = findQueueFamilies(device);
			if (deviceSuitable(device, m_queue_families)) {
				m_physical_device = device;
				break;
			}
		}

		return m_physical_device == VK_NULL_HANDLE ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	int Window::createLogicalDevice() {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = m_queue_families.graphics.value();
		queue_create_info.queueCount = 1;

		float queue_priority = 1.0f;
		queue_create_info.pQueuePriorities = &queue_priority;

		VkPhysicalDeviceFeatures device_features{};

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.queueCreateInfoCount = 1;

		create_info.pEnabledFeatures = &device_features;
		create_info.enabledLayerCount = 0;

		if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}



}