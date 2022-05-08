#ifndef _FUJI_WINDOW_H_
#define _FUJI_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <optional>
#include <vector>
#include <chrono>

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

		void draw();

	private:
		const int m_width = 800;
		const int m_height = 600;
		std::string m_title{};

		std::chrono::steady_clock::time_point m_last_frame_time = std::chrono::high_resolution_clock::now();
		float m_delta = 0.0f;

		int pickPhysicalDevice();
		int createLogicalDevice();
		int createSwapChain();
		int createImageViews();
		int createRenderPass();
		int createGraphicsPipeline();
		int createFramebuffers();
		int createCommandPool();
		int createCommandBuffer();
		int createSyncObjects();

		int recordCommandBuffer(uint32_t);

		GLFWwindow* m_window = nullptr;
		VkInstance m_instance{};
		VkSurfaceKHR m_surface{};

		VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;

		QueueFamilyIndices m_queue_families{};
		SwapChainSupportDetails m_sc_support_details{};
		VkQueue m_graphics_queue = VK_NULL_HANDLE;
		VkQueue m_present_queue = VK_NULL_HANDLE;
		VkCommandPool m_command_pool = VK_NULL_HANDLE;
		VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;

		VkSwapchainKHR m_swap_chain = VK_NULL_HANDLE;
		std::vector<VkImage> m_swap_chain_images;
		std::vector<VkImageView> m_swap_chain_image_views;
		std::vector<VkFramebuffer> m_swap_chain_framebuffers;
		VkFormat m_swap_chain_image_format{};
		VkExtent2D m_swap_chain_extent{};

		VkRenderPass m_render_pass = VK_NULL_HANDLE;
		VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
		VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

		VkSemaphore m_image_available = VK_NULL_HANDLE;
		VkSemaphore m_rendering_finished = VK_NULL_HANDLE;
		VkFence m_in_flight = VK_NULL_HANDLE;

	};
}


#endif
