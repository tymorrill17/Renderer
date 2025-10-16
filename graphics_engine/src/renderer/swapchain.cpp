#include "renderer/swapchain.h"
#include "renderer/image.h"
#include "utility/logger.h"
#include <cstdint>
#include <stdexcept>

void Swapchain::initialize(Device* device, Window* window) {
    this->device = device;
    this->window = window;
    window_resized = false;
    image_index = 0;
    create_swapchain();
}

void Swapchain::create_swapchain() {

	// Query swapchain support details
	SwapchainSupportDetails support_details = query_swapchain_support(device->physical_device, device->window_surface);

    bool physical_device_adequate = !support_details.formats.empty() && !support_details.present_modes.empty();
    if (!physical_device_adequate) {
        Logger::logError("The physical device does not have sufficient swapchain support!");
    }

	// Select the format and present modes of the swapchain
	VkSurfaceFormatKHR surface_format = select_swapchain_surface_format(support_details.formats);
	VkPresentModeKHR present_mode = select_swapchain_present_mode(support_details.present_modes);

    extent = find_swapchain_extent(support_details.capabilities, window);
	image_format = surface_format.format;
    frames_in_flight = support_details.capabilities.minImageCount + 1; // Set how many images will be in the swapchain. Typically this will be 2.

    // Cap the frames in flight to the max swapchain image count
	if (support_details.capabilities.maxImageCount > 0 && frames_in_flight > support_details.capabilities.maxImageCount)
		frames_in_flight = support_details.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchain_create_info{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = device->window_surface,
		.minImageCount = frames_in_flight,
		.imageFormat = image_format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1, // number of layers in each image. This will usually be one unless doing something like stereoscopic 3D
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // We will be rendering to a draw image then transferring to the swapchain image
		.preTransform = support_details.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
	};

    // Get graphics and present queue indices
	const QueueFamilyIndices indices = device->queue_indices;
	uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

    // Determine how to handle images across queue families
	// Are there >1 queue families?
	if (indices.graphics_family != indices.present_family) {
		// If so, use concurrent mode. It is faster to transfer since no queue families own the image
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else {
		// Otherwise, use exclusive mode, as it is faster
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
	}

	if (vkCreateSwapchainKHR(device->logical_device, &swapchain_create_info, nullptr, &handle) != VK_SUCCESS) {
        Logger::logError("Failed to create swapchain!");
	}

	// Get the new swapchain's images
    std::vector<VkImage> new_images;
	vkGetSwapchainImagesKHR(device->logical_device, handle, &frames_in_flight, nullptr);
	new_images.resize(frames_in_flight);
	vkGetSwapchainImagesKHR(device->logical_device, handle, &frames_in_flight, new_images.data());

	// Now fill the images vector, which creates the image views through the Image constructor
	images.reserve(frames_in_flight);
	VkExtent3D swapchain_image_extent{ extent.width, extent.height, 1 };
	for (auto image : new_images) {
        SwapchainImage temp_image;
        temp_image.initialize(device, image, swapchain_image_extent, image_format);
		images.push_back(temp_image);
	}
}

void Swapchain::cleanup() {
	vkDestroySwapchainKHR(device->logical_device, handle, nullptr);
    for (auto& image : images) {
        image.cleanup();
    }
    images.clear();
}

void Swapchain::recreate() {
    vkDeviceWaitIdle(device->logical_device);
	cleanup(); // Destroy old swapchain
	create_swapchain(); // Recreate the swapchain
    window_resized = false;
}

void Swapchain::acquire_next_image(FrameSync* sync) {
    VkResult e = vkAcquireNextImageKHR(device->logical_device, handle, 1000000000, sync->present_semaphore.handle, nullptr, &image_index);
    if (e == VK_ERROR_OUT_OF_DATE_KHR) { // This is a point of entry for the information that the window has been resized.
        window_resized = true;
    } else if (e != VK_SUCCESS) {
        Logger::logError("Failed to acquire next swapchain image!");
    }

}

void Swapchain::present_to_screen(VkQueue queue, FrameSync* sync) {
    VkSemaphore wait_semaphore = sync->render_semaphore.handle;
    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &wait_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &handle,
            .pImageIndices = &image_index
    };
    VkResult e = vkQueuePresentKHR(queue, &present_info);
    if (e == VK_ERROR_OUT_OF_DATE_KHR) { // This is a point of entry for the information that the window has been resized.
        window_resized = true;
    } else if (e != VK_SUCCESS) {
        Logger::logError("Failed to present to screen!");
    }
}

SwapchainSupportDetails Swapchain::query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapchainSupportDetails details;

	// Get the capabilities of both the device and the surface
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Get the supported formats
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
	if (format_count > 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
	}

	// Get the supported present modes
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
	if (present_mode_count > 0) {
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
	}

	return details;
}

VkExtent2D Swapchain::find_swapchain_extent(VkSurfaceCapabilitiesKHR capabilities, Window* window) {

	// In Vulkan, an extent of size UINT32_MAX means the window resolution should be used
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else { // Otherwise, the window manager allows a custom resolution
		VkExtent2D window_extent = window->extent;

		// Truncates the extent to within the surface capabilities
		window_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, window_extent.width));
		window_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, window_extent.height));

		return window_extent;
	}
}

VkSurfaceFormatKHR Swapchain::select_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    // Try to find support for 8-bit SRGB color format
    for (const auto& format : available_formats) {
        if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    // Otherwise, default to the first available
    return available_formats[0];
}

VkPresentModeKHR Swapchain::select_swapchain_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    // Try to find MAILBOX present mode
    for (const auto& mode : available_present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    // Otherwise, default to FIFO mode
    return VK_PRESENT_MODE_FIFO_KHR;
}

