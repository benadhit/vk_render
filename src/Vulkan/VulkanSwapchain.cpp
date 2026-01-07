#include "VulkanSwapchain.h"
#include "../VulkanUtils.h"
void VulkanSwapchain::init(VkPhysicalDevice physicalDevice, 
    VkDevice device, 
    VkSurfaceKHR surface,
    uint32_t graphicsQueueFamilyIndex, 
    uint32_t presentQueueFamilyIndex)
{
    SwapchainSupportDetails details = fetchSwapchainSupportDetails(physicalDevice, device, surface);
    SwapchainSettigs setting = selectOptimalSwapchainSetting(details);
    swapchain_ = createSwapchain(physicalDevice, 
        device, 
        surface, 
        details, 
        setting, 
        graphicsQueueFamilyIndex, 
        presentQueueFamilyIndex);
    uint32_t swapchianImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain_, &swapchianImageCount, nullptr));
    swapchainImages_.resize(swapchianImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain_, &swapchianImageCount, swapchainImages_.data()));
    swapchainImageViews_.resize(swapchianImageCount);
    for (size_t i =0; i < swapchainImageViews_.size(); ++i) 
    {
        swapchainImageViews_[i] = createImageView(device, 
            swapchainImages_[i], 
            setting.format.format);
    }
}

void VulkanSwapchain::destroy(VkDevice device)
{
    for (auto& imageView: swapchainImageViews_) {
        vkDestroyImageView(device, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(device, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
}