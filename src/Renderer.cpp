#include "Renderer.h"

void Renderer::init(VkDevice device, const char* vertSpv, const char* fragSpv)
{
    device_ = device;
    vertShader_ = createShaderModule(device, vertSpv);
    fragShader_ = createShaderModule(device_, fragSpv);

}

void Renderer::shutdown()
{
    vkDestroyShaderModule(device_, vertShader_, nullptr);
    vkDestroyShaderModule(device_, fragShader_, nullptr);
}