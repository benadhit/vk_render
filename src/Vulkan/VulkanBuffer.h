#pragma once
#include "VulkanInternal.h"

class VulkanBuffer {
public:
    VulkanBuffer();
    ~VulkanBuffer();

    void init(VkDevice device, 
        VkPhysicalDeviceMemoryProperties memoryProperties,
        VkBufferUsageFlags usage,
        size_t size);

    void destroy(VkDevice device);

private:
    VkBufferUsageFlags usage_;
    size_t           bufferSize_;
    VkBuffer         buffer_;
    VkDeviceMemory   bufferMemory_;
};