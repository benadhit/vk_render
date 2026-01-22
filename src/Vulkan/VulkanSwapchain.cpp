#include "VulkanSwapchain.h"
#include "../VulkanUtils.h"
void VulkanSwapchain::init(VkPhysicalDevice physicalDevice, 
    VkDevice device, 
    VkSurfaceKHR surface,
    VkRenderPass renderPass,
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
    images_.resize(swapchianImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain_, &swapchianImageCount, images_.data()));
    imageViews_.resize(swapchianImageCount);

    for (size_t i = 0; i < imageViews_.size(); ++i) 
    {
        imageViews_[i] = createImageView2D(device, 
            images_[i], 
            VK_IMAGE_ASPECT_COLOR_BIT,
            setting.format.format);
    }

    framebuffers_.resize(imageViews_.size());
    for (uint32_t i = 0; i < framebuffers_.size(); ++ i) {
        VkImageView imageView[] = { 
            imageViews_[i]
        };
        framebuffers_[i] = createFrambuffer(device, renderPass, 
            extent_,
            imageView,
            ARRAY_SIZE(imageView)
        );
    }
}

void VulkanSwapchain::destroy(VkDevice device)
{
    for (auto& imageView: imageViews_) {
        vkDestroyImageView(device, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(device, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
}