#ifndef VULKAN_UTILS_H__
#define VULKAN_UTILS_H__
#include <stdint.h>
#include <vector>
#define VK_CHECK(call)                  \
    do{                                 \
        VkResult result = call;         \
        assert(result == VK_SUCCESS);   \
    }while(0)

bool checkRequireExtensions(const std::vector<const char*>& requiredExtensions);
bool checkRequiredLayerExtension(const std::vector<const char*>& requiredLayers);

#endif