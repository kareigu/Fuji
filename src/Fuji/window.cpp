#include "window.h"

#include <fmt/core.h>


#include <vector>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <filesystem>




namespace fuji {

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families) {
			if (indices.ready()) {
				fmt::print("All queue_family indices found\n");
				break;
			}

			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphics = i;
				fmt::print("Found graphics queue index: {:d}\n", i);

				VkBool32 present_support = false;
				(void)vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
				if (present_support) {
					indices.present = i;
					fmt::print("Found presentation queue index: {:d}\n", i);
				}
			}

			i++;
		}

		return indices;
	}

	static const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensions_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extensions_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensions_count, available_extensions.data());

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

		for (const auto& extension : available_extensions) {
			if (required_extensions.erase(extension.extensionName) > 0)
				fmt::print("Required extension {:s} v{:d} available\n", extension.extensionName, extension.specVersion);
		}

		return required_extensions.empty();
	}

	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
		for (const auto& format : available_formats) {
			if (
				format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR
				) {
				return format;
			}
		}

		return available_formats[0];
	}

	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
		for (const auto& mode : available_present_modes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	static VkExtent2D chooseSwapExtent(GLFWwindow* const window, const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height),
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	int Window::createSwapChain() {
		auto surface_format = chooseSwapSurfaceFormat(m_sc_support_details.formats);
		auto present_mode = chooseSwapPresentMode(m_sc_support_details.present_modes);
		auto extent = chooseSwapExtent(m_window, m_sc_support_details.capabilities);

		fmt::print("Got VkSurfaceFormatKHR:\n  format = {},\n  colorSpace = {}\n", 
			surface_format.format == VK_FORMAT_B8G8R8A8_SRGB ? "VK_FORMAT_B8G8R8A8_SRGB" : "Device default", 
			surface_format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR ? "VK_COLORSPACE_SRGB_NONLINEAR_KHR" : "Device default");
		fmt::print("Got VkPresentModeKHR: {}\n", present_mode == VK_PRESENT_MODE_MAILBOX_KHR ? "VK_PRESENT_MODE_MAILBOX_KHR" : "VK_PRESENT_MODE_FIFO_KHR");
		fmt::print("Got Extent2D(width = {}, height = {})\n", extent.width, extent.height);

		uint32_t image_count = m_sc_support_details.capabilities.minImageCount + 1;
		auto max_image_count = m_sc_support_details.capabilities.maxImageCount;
		if (max_image_count > 0 && image_count > max_image_count)
			image_count = max_image_count;


		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = m_surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (m_queue_families.graphics != m_queue_families.present) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			uint32_t family_indices[] = { m_queue_families.graphics.value(), m_queue_families.present.value() };
			create_info.pQueueFamilyIndices = family_indices;
		}
		else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		create_info.preTransform = m_sc_support_details.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		create_info.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swap_chain) != VK_SUCCESS)
			return EXIT_FAILURE;

		vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, nullptr);
		m_swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(m_device, m_swap_chain, &image_count, m_swap_chain_images.data());

		fmt::print("Stored swapchain images to m_swap_chain_images\n");
		m_swap_chain_image_format = surface_format.format;
		m_swap_chain_extent = extent;

		return EXIT_SUCCESS;
	}

	int Window::createImageViews() {
		m_swap_chain_image_views.resize(m_swap_chain_images.size());

		for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = m_swap_chain_images[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = m_swap_chain_image_format;

			create_info.components = { 
				VK_COMPONENT_SWIZZLE_IDENTITY, // R
				VK_COMPONENT_SWIZZLE_IDENTITY, // G
				VK_COMPONENT_SWIZZLE_IDENTITY, // B
				VK_COMPONENT_SWIZZLE_IDENTITY, // A
			};

			create_info.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask
				0,												 // baseMipLevel
				1,												 // levelCount
				0,												 // baseArrayLayer
				1,												 // layerCount
			};

			if (vkCreateImageView(m_device, &create_info, nullptr, &m_swap_chain_image_views[i]) != VK_SUCCESS) {
				fmt::print("Couldn't create VkImageViews for index {}({:p})\n", i, (void*)&m_swap_chain_images[i]);
				return EXIT_FAILURE;
			}
		}

		return EXIT_SUCCESS;
	}

	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formats_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats_count, nullptr);
		if (formats_count > 0) {
			details.formats.resize(formats_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats_count, details.formats.data());
		}

		uint32_t present_modes_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, nullptr);
		if (present_modes_count > 0) {
			details.present_modes.resize(present_modes_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes_count, details.present_modes.data());
		}

		return details;
	}

	static bool deviceSuitable(VkPhysicalDevice device, QueueFamilyIndices queue_family_indices, SwapChainSupportDetails swapchain_details) {
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceProperties(device, &device_properties);
		vkGetPhysicalDeviceFeatures(device, &device_features);

		bool properties_and_features = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;

		bool extensions_supported = checkDeviceExtensionSupport(device);

		bool swapchain_supported = swapchain_details.ready();
		swapchain_supported ? fmt::print("Swapchain supported\n") : fmt::print("Swapchain not supported\n");

		return queue_family_indices.ready() && properties_and_features && extensions_supported && swapchain_supported;
	}


	static std::vector<char> readFile(const std::string& filename) {
#ifdef DEBUG
		auto current_path = std::filesystem::current_path();
		auto file_path = current_path.append(fmt::format("src/Fuji/{}", filename)).generic_string();
		fmt::print("DEBUG file path {:s}\n", file_path);
		std::ifstream file(file_path, std::ios::ate | std::ios::binary);
#else
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
#endif

		
		if (!file.is_open()) {

#ifdef DEBUG
			fmt::print("Failed opening file: {:s}\n", file_path);
#else
			fmt::print("Failed opening file: {:s}\n", filename);
#endif
			return {};
		}

		size_t file_size = (size_t)file.tellg();
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);


		fmt::print("Read file: {:s}\n", filename);
		return buffer;
	}

	static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& data) {
		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = data.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(data.data());

		VkShaderModule module;
		if (vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS)
			return nullptr;

		return module;
	}


#define CREATE_SHADER_MODULE(shader_name) createShaderModule(m_device, shader_name); \
	if (shader_name##_module == nullptr) { \
		fmt::print("Couldn't create VkShaderModule for {:s}\n", #shader_name); \
		return EXIT_FAILURE; \
	} \
	fmt::print("Created VkShaderModule for {:s}\n", #shader_name)

	int Window::createGraphicsPipeline() {
		auto vert_shader = readFile("shaders/triangle.vert.spv");
		auto frag_shader = readFile("shaders/triangle.frag.spv");

		auto vert_shader_module = CREATE_SHADER_MODULE(vert_shader);
		auto frag_shader_module = CREATE_SHADER_MODULE(frag_shader);

		VkPipelineShaderStageCreateInfo vert_shader_create_info{};
		vert_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_create_info.module = vert_shader_module;
		vert_shader_create_info.pName = "main";
		vert_shader_create_info.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo frag_shader_create_info{};
		frag_shader_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_create_info.module = frag_shader_module;
		frag_shader_create_info.pName = "main";
		frag_shader_create_info.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_create_info, frag_shader_create_info };

		VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
		vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_create_info.vertexBindingDescriptionCount = 0;
		vertex_input_create_info.pVertexBindingDescriptions = nullptr;
		vertex_input_create_info.vertexAttributeDescriptionCount = 0;
		vertex_input_create_info.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
		input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swap_chain_extent.width);
		viewport.height = static_cast<float>(m_swap_chain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swap_chain_extent;

		VkPipelineViewportStateCreateInfo viewport_state_create_info{};
		viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create_info.viewportCount = 1;
		viewport_state_create_info.pViewports = &viewport;
		viewport_state_create_info.scissorCount = 1;
		viewport_state_create_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer_create_info{};
		rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer_create_info.depthClampEnable = VK_FALSE;
		rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
		rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer_create_info.lineWidth = 1.0f;
		rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer_create_info.depthBiasEnable = VK_FALSE;
		rasterizer_create_info.depthBiasConstantFactor = 0.0f;
		rasterizer_create_info.depthBiasClamp = 0.0f;
		rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling_create_info{};
		multisampling_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling_create_info.sampleShadingEnable = VK_FALSE;
		multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling_create_info.minSampleShading = 1.0f;
		multisampling_create_info.pSampleMask = nullptr;
		multisampling_create_info.alphaToCoverageEnable = VK_FALSE;
		multisampling_create_info.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blend_create_info{};
		color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_create_info.logicOpEnable = VK_TRUE;
		color_blend_create_info.logicOp = VK_LOGIC_OP_AND;
		color_blend_create_info.attachmentCount = 1;
		color_blend_create_info.pAttachments = &color_blend_attachment;
		color_blend_create_info.blendConstants[0] = 0.0f;
		color_blend_create_info.blendConstants[1] = 0.0f;
		color_blend_create_info.blendConstants[2] = 0.0f;
		color_blend_create_info.blendConstants[3] = 0.0f;


		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH,
		};

		VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state_create_info.pDynamicStates = dynamic_states.data();

		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = nullptr;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_device, &pipeline_layout_create_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
			fmt::print("Couldn't create VkPipelineLayout\n");
			return EXIT_FAILURE;
		}

		fmt::print("Created VkPipelineLayout({:p})\n", (void*)m_pipeline_layout);

		VkGraphicsPipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = shader_stages;
		pipeline_create_info.pVertexInputState = &vertex_input_create_info;
		pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		pipeline_create_info.pViewportState = &viewport_state_create_info;
		pipeline_create_info.pRasterizationState = &rasterizer_create_info;
		pipeline_create_info.pMultisampleState = &multisampling_create_info;
		pipeline_create_info.pDepthStencilState = nullptr;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_state_create_info;
		pipeline_create_info.layout = m_pipeline_layout;
		pipeline_create_info.renderPass = m_render_pass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_create_info.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &m_graphics_pipeline) != VK_SUCCESS) {
			fmt::print("Couldn't create VkPipeline\n");
			return EXIT_FAILURE;
		}

		vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
		vkDestroyShaderModule(m_device, frag_shader_module, nullptr);

		return EXIT_SUCCESS;
	}

	int Window::createRenderPass() {
		VkAttachmentDescription color_attachment{};
		color_attachment.format = m_swap_chain_image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkRenderPassCreateInfo render_pass_create_info{};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments = &color_attachment;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass;

		if (vkCreateRenderPass(m_device, &render_pass_create_info, nullptr, &m_render_pass) != VK_SUCCESS) {
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}


	bool Window::shouldRun() {
		return m_window ? !glfwWindowShouldClose(m_window) : false;
	}

	void Window::poll() {
		glfwPollEvents();
	}

	void Window::close() {

		uint32_t destroyed_image_views = 0;
		for (auto image_view : m_swap_chain_image_views) {
			vkDestroyImageView(m_device, image_view, nullptr);
			destroyed_image_views++;
		}
		fmt::print("Destroyed {:d} VkImageViews\n", destroyed_image_views);


		vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
		fmt::print("Destroyed VkPipeline\n");

		vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
		fmt::print("Destroyed VkPipelineLayout\n");

		vkDestroyRenderPass(m_device, m_render_pass, nullptr);
		fmt::print("Destroyed VkRenderPass\n");

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
			return EXIT_FAILURE;
		}

		fmt::print("Created VkDevice({:p})\n", (void*)&m_device);

		vkGetDeviceQueue(m_device, m_queue_families.graphics.value(), 0, &m_graphics_queue);
		fmt::print("Got m_graphics_queue handle ({:p})\n", (void*)&m_graphics_queue);
		vkGetDeviceQueue(m_device, m_queue_families.present.value(), 0, &m_present_queue);
		fmt::print("Got m_present_queue handle ({:p})\n", (void*)&m_present_queue);

		if (createSwapChain()) {
			fmt::print("Failed creating swapchain\n");
			return EXIT_FAILURE;
		}

		fmt::print("Created VkSwapchainKHR({:p}) with {:d} images\n", (void*)m_swap_chain, m_swap_chain_images.size());

		if (createImageViews()) {
			fmt::print("Failed creating VkImageViews\n");
			return EXIT_FAILURE;
		}

		fmt::print("Created {:d} VkImageViews\n", m_swap_chain_image_views.size());

		if (createRenderPass()) {
			fmt::print("Failed creating VkRenderPass\n");
			return EXIT_FAILURE;
		}

		fmt::print("Created VkRenderPass({:p})\n", (void*)m_render_pass);

		if (createGraphicsPipeline()) {
			fmt::print("Failed creating VkPipeline\n");
			return EXIT_FAILURE;
		}

		fmt::print("Created VkPipeline({:p}) for graphics\n", (void*)m_graphics_pipeline);

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
			FMT_ASSERT(m_surface != VK_NULL_HANDLE, "Surface not present during piccking VkPhysicalDevice");
			m_queue_families = findQueueFamilies(device, m_surface);
			m_sc_support_details = querySwapChainSupport(device, m_surface);
			if (deviceSuitable(device, m_queue_families, m_sc_support_details)) {
				m_physical_device = device;
				break;
			}
		}

		return m_physical_device == VK_NULL_HANDLE ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	int Window::createLogicalDevice() {

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_family_count = {
			m_queue_families.graphics.value(),
			m_queue_families.present.value(),
		};

		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_family_count) {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}


		VkPhysicalDeviceFeatures device_features{};

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();

		create_info.pEnabledFeatures = &device_features;
		create_info.enabledLayerCount = 0;

		create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		create_info.ppEnabledExtensionNames = device_extensions.data();


		if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}



}