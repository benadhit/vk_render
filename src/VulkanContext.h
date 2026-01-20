#pragma once
#include "VulkanUtils.h"

class VulkanContext {
public:
    VulkanContext();
    VkSemaphore createSemaphore();
    VkBuffer createBuffer();
    VkFence createFence();
    VkShaderModule createShaderModule(const char* spvPath);
    VkImageView createImageView(VkImage image, VkFormat format);
    VkCommandPool createCommandPool(uint32_t queueFamilyIndex);
    VkCommandBuffer createCommandBuffer(VkCommandPool commandPool);
    VkDeviceMemory createBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags memoryPropertyFlags);
private:
    VkInstance instance_{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
    VkPhysicalDeviceMemoryProperties memoryProperties{};
};