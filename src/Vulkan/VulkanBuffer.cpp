#include "Vulkan/VulkanBuffer.h"

VulkanBuffer::VulkanBuffer()
{
    
}

VulkanBuffer::~VulkanBuffer()
{

}

void VulkanBuffer::init(VkDevice device, 
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkBufferUsageFlags usage,
    size_t size)
{
    bufferSize_ = size;
    usage_ = usage;
    buffer_ = createBuffer(device, usage, size);
    bufferMemory_ = createBufferMemory(device, 
        buffer_, 
        memoryProperties, 
        VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(device, buffer_, bufferMemory_, 0);
}

void VulkanBuffer::destroy(VkDevice device)
{
    vkDestroyBuffer(device, buffer_, nullptr);
    vkFreeMemory(device, bufferMemory_, nullptr);
    bufferSize_ = 0;
}