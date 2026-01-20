#pragma once
#include "../VulkanUtils.h"
class VulkanBuffer {
public:
    enum class BufferType { 
        DeviceLocal,
        SharedByCpuAndGpu,
    };


private:
    BufferType type_;
    VkBuffer buffer_;
    VkDeviceMemory bufferMemory_;
};