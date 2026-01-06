#include "VulkanInstance.h"
#include <utility>
VulkanInstanceBuilder::VulkanInstanceBuilder() = default;

VulkanInstance::VulkanInstance(VkInstance instance)
    : instance_(instance)
{}

VulkanInstance:: ~VulkanInstance()
{
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
    : instance_(other.instance_)
{
    other.instance_  = VK_NULL_HANDLE;
}

VulkanInstance& VulkanInstance::operator=(VulkanInstance&& other) noexcept
{
    VulkanInstance tmp = std::move(other);
    std::swap(tmp.instance_, instance_);
    return *this;
}

VulkanInstanceBuilder VulkanInstance::getBuilder()
{
    return VulkanInstanceBuilder();
}

//============================================================================


VulkanInstanceBuilder& VulkanInstanceBuilder::addExtension(const char* extension)
{
    extensions_.push_back(extension);
    return *this;
}

VulkanInstanceBuilder& VulkanInstanceBuilder::addLayer(const char* layer)
{
    layers_.push_back(layer);
    return *this;
}

VulkanInstance VulkanInstanceBuilder::build()
{
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pEngineName = nullptr;
    appInfo.applicationVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.pApplicationName = "PBR SandBox";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo info{};
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extensions_.size();
    instanceInfo.ppEnabledExtensionNames = extensions_.empty() ? nullptr : extensions_.data();
    instanceInfo.ppEnabledLayerNames = layers_.empty() ? nullptr : layers_.data();
    instanceInfo.enabledLayerCount = layers_.size();
    VkInstance instance {VK_NULL_HANDLE};
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));
    return VulkanInstance(instance);
}