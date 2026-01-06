#ifndef HAMON_VULKAN_INSTANCE_H__
#define HAMON_VULKAN_INSTANCE_H__
#include "VulkanInternal.h"
#include <vector>
class VulkanInstance;

class VulkanInstanceBuilder {
public:
    VulkanInstanceBuilder();
    VulkanInstanceBuilder& addExtension(const char* extension);
    VulkanInstanceBuilder& addLayer(const char* layer);
    VulkanInstance build();
private:
    std::vector<const char*> extensions_;
    std::vector<const char*> layers_;
};

class VulkanInstance {
public:
    VulkanInstance(VkInstance instance);
    ~VulkanInstance();
    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&& other) noexcept;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance& operator= (VulkanInstance&&) noexcept;
    
    operator VkInstance() const 
    {
        return instance_;
    }

    static VulkanInstanceBuilder getBuilder();
private:
    VkInstance instance_;
};

#endif
