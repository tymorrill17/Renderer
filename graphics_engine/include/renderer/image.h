#pragma once
#include "vulkan/vulkan.h"
#include "vma/vk_mem_alloc.h"
#include "utility/allocator.h"
#include "renderer/command.h"
#include "utility/logger.h"
#include "NonCopyable.h"

// Base image class
class Image : public NonCopyable {
public:
	Image(VkImage image = VK_NULL_HANDLE, VkImageView imageView = VK_NULL_HANDLE,
		VkExtent3D extent = { 0, 0, 0 }, VkFormat format = VK_FORMAT_UNDEFINED,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED);
	virtual ~Image() = 0;

    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

	// @brief Transitions image from currentLayout to newLayout
	// @param cmd - Command buffer to submit the barrier to (The barrier performs the transition)
	// @param image - The image to transition
	// @param currentLayout - Current image layout
	// @param newLayout - Desired image layout to transition to
	void transitionImage(Command& cmd, VkImageLayout newLayout);

    inline VkImage image() { return _image; }
    inline VkImageView imageView() { return _imageView; }
    inline VkImageLayout imageLayout() { return _imageLayout; }
    inline VkExtent3D extent() { return _extent; }
    inline VkFormat format() { return _format; }

	// STATIC METHODS

	// @brief Copies image src into image dst on the GPU. Uses blit to copy the images
	// @param cmd - Command buffer to submit the copy to
	// @param src - Image to be copied
	// @param dst - destination image to copy to
	static void copyImageOnGPU(Command& cmd, Image* src, Image* dst);

	// @brief Populates a VkRenderingAttachmentInfo struct needed in order to begin rendering without a renderpass.
	//	      Normally, the renderpass contains information about the attachments, but we are using renderpass-less dynamic rendering
	// @param imageView - Image view of the associated image
	// @param pClear - Clear value. This may be null for some cases
	// @param imageLayout - Image layout that the associated image will be in when rendering starts
	// @return The populated VkRenderingAttachmentInfoKHR struct
	static VkRenderingAttachmentInfoKHR attachmentInfo(VkImageView imageView, VkClearValue* pClear, VkImageLayout imageLayout);

protected:
	VkImage _image;
	VkImageView _imageView;
	VkImageLayout _imageLayout;
	VkExtent3D _extent;
	VkFormat _format;
};

class AllocatedImage : public Image {
public:
	// @brief AllocatedImage constructor. It also allocates memory for the image using VMA and creates an image view
	// @param device - Vulkan logical device
	// @param allocator - VMA allocator object
	// @param extent - size of the image. Images can be 3D
	// @param format - format of the image
	// @param usageFlags - How this image will be used by Vulkan
	// @param memoryUsage - VMA flag for where the image will be allocated to
	// @param vkMemoryUsage - Vulkan flag for where the image will be allocated. Should match the memoryUsage flags
	// @param aspectFlags - Image aspect flags for image view creation
	AllocatedImage(Device* device, DeviceMemoryManager* deviceMemoryManager,
		VkExtent3D extent, VkFormat format, VkImageUsageFlags usageFlags,
		VmaMemoryUsage memoryUsage, VkMemoryAllocateFlags vkMemoryUsage,
		VkImageAspectFlags aspectFlags);
    ~AllocatedImage() override { cleanup(); }

    AllocatedImage(AllocatedImage&& other) noexcept;
    AllocatedImage& operator=(AllocatedImage&& other) noexcept;

	// @brief Recreates the image for when the window is resized
	void recreate(VkExtent3D extent);

protected:
	Device* _device;
	DeviceMemoryManager* _deviceMemoryManager;
	VmaAllocation _allocation;
	VkImageUsageFlags _usageFlags;
	VmaMemoryUsage _memoryUsage;
	VkMemoryAllocateFlags _vkMemoryUsage;
	VkImageAspectFlags _aspectFlags;

    void createAllocatedImage();
    void cleanup();
};

class SwapchainImage : public Image {
public:
    // @brief Creates an image view for the swapchain image
    // @param device - Vulkan logical device
    // @param image - The swapchain creates its own swapchain images on creation, so pass those here
    // @param extent - size of the image
    // @param format - format of the image
	SwapchainImage(Device* device, VkImage image, VkExtent3D extent, VkFormat format);
    ~SwapchainImage() override;

    SwapchainImage(SwapchainImage&& other) noexcept;
    SwapchainImage& operator=(SwapchainImage&& other) noexcept;

protected:
	Device* _device;

};
