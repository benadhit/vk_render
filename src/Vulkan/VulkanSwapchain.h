#ifndef HAMON_VULKAN_SWAPCHAIN_H__
#define HAMON_VULKAN_SWAPCHAIN_H__
#include "VulkanInternal.h"

class VulkanSwapchain {
public:
    VulkanSwapchain() = default;
    void init(VkPhysicalDevice physicalDevice,
        VkDevice device, 
        VkSurfaceKHR surface, 
        uint32_t graphicsQueueFamilyIndex, 
        uint32_t presentQueueFamilyIndex);
    void destroy(VkDevice device);
private:
    uint32_t width;
    uint32_t height;
    uint32_t swapchianImageCount = 0;
    VkSwapchainKHR  swapchain_{VK_NULL_HANDLE};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> swapchainFramebuffers_;
};

#endif