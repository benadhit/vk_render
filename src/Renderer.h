#pragma once
#include "VulkanUtils.h"
class Renderer {
public:
    Renderer() = default;
    void init(VkDevice device, 
        const char* vertSpv, 
        const char* fragShader,
        uint32_t width,
        uint32_t height);

    void destroy();
private:
    VkDevice device_{VK_NULL_HANDLE};
    VkShaderModule vertShader_;
    VkShaderModule fragShader_;
};