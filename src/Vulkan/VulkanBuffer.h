#pragma once
#include "../VulkanUtils.h"
class VulkanBuffer {
public:
    enum class BufferStorageType { 
        DeviceLocal,
        SharedByCpuAndGpu,
    };

    enum class BufferUsageType{ 
        Vertex,
        Index,
        Uniform,
    };


private:
    BufferStorageType storageType_;
    BufferUsageType  usageType_;
    size_t           bufferSize_;
    VkBuffer         buffer_;
    VkDeviceMemory  bufferMemory_;
};