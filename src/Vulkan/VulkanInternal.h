#ifndef HAMON_VULKAN_INTERNAL_H__
#define HAMON_VULKAN_INTERNAL_H__
#include "VulkanUtils.h"
#include <assert.h>
#include <vector>
#include <string>
#include <unordered_map>
#define VK_CHECK(call)                  \
    do{                                 \
        VkResult result = call;         \
        assert(result == VK_SUCCESS);   \
    }while(0)

template <typename T>
class VulkanObject{

};
#endif