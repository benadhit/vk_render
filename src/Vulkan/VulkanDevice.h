#ifndef HAMON_VULKAN_DEVICE_H__
#define HAMON_VULKAN_DEVICE_H__
#include "VulkanInternal.h"

class VulkanDeviceBuilder {
public:
    VulkanDevice build(VkPhysicalDevice physicalDevice);
private:

};

class VulkanDevice {
public:
    VulkanDevice() = default;
    VulkanDevice(VkPhysicalDevice physicalDevice, 
        VkDevice device);
    VulkanDevice(const VulkanDevice& device) = delete;
    VulkanDevice(VulkanDevice&& device) noexcept;

    VulkanDevice& operator=(const VulkanDevice& device) = delete;
    VulkanDevice& operator=(VulkanDevice&& device) noexcept;

private:
    VkPhysicalDevice physicalDevice;
    VkDevice device;
};

#endif