#ifndef HAMON_VULKAN_SWAPCHAIN_H__
#define HAMON_VULKAN_SWAPCHAIN_H__
#include "VulkanInternal.h"

class VulkanSwapchain {
public:
    VulkanSwapchain() = default;
    ~VulkanSwapchain();
    VulkanSwapchain(const VulkanSwapchain&)  = delete;
    VulkanSwapchain(VulkanSwapchain&&) noexcept;

    VulkanSwapchain& operator=(const VulkanSwapchain&)  = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&) noexcept;

    void init(VkPhysicalDevice physicalDevice,
        VkDevice device, 
        VkSurfaceKHR surface, 
        VkRenderPass renderPass,
        uint32_t graphicsQueueFamilyIndex, 
        uint32_t presentQueueFamilyIndex);

    void destroy(VkDevice device);
private:
    VkExtent2D      extent_;
    uint32_t        imageCount_ = 0;
    uint32_t        maxFrameInFlights_;
    VkSwapchainKHR  swapchain_{VK_NULL_HANDLE};
    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
    std::vector<VkFramebuffer> framebuffers_;
};

#endif