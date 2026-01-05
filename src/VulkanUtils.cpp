#include "VulkanUtils.h"
#include "vulkan.h"
#include <iostream>

bool checkRequireExtensions(const std::vector<const char*>& requiredExtensions)
{
    VkExtensionProperties extensions[128];
    uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    std::cout << "Available Extensions:\n"; 
    for (uint32_t idx = 0; idx < requiredExtensions.size(); ++idx)
    {
        bool supported = false;
        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            if (strcmp(requiredExtensions[i], extensions[idx].extensionName) != 0) {
                supported = true;
                break;
            }
        }
        if (!supported)
        {
            return false;
        }

        std::cout << "\t" << extensions[idx].extensionName
                << "\t" << extensions[idx].specVersion 
                << "\t" << (supported ? "[GLFW Extension]" : "") << "\n";
    }
    return true;
}

bool checkRequiredLayerExtension(const std::vector<const char*>& requiredLayers)
{
    VkLayerProperties layers[128];
    uint32_t layerCount = sizeof(layers) / sizeof(layers[0]);
    vkEnumerateInstanceLayerProperties(&layerCount, layers);
    for (size_t ridx = 0; ridx < requiredLayers.size(); ++ridx )
    {
        bool supported = false;
        for (uint32_t idx = 0; idx < layerCount; ++idx)
        {
            if (strcmp(requiredLayers[ridx], layers[idx].layerName) != 0) {
                supported = true;
                break;
            }
        }
        if (!supported)
        {
            return false;
        }
        // std::cout << "\t" << layers[ridx].layerName
        // << "\t" << layer[ridx].specVersion 
        // << "\t" << (supported ? "[GLFW Extension]" : "") << "\n";
    }
    return true;
}