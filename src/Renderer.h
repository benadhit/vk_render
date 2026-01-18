#pragma once
#include "VulkanUtils.h"

struct RendererContext {
    VkExtent2D extent_;
    VkFormat format_;
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue_;
    VkDevice device_{VK_NULL_HANDLE};
    VkSwapchainKHR  swapchain_;
    std::vector<VkImageView>   imageViews_;
};

class Renderer {
public:
    Renderer() = default;

    Renderer(const RendererContext& context);
    void init(const char* vertSpv, 
        const char* fragShader);

    void frameStart();

    void render();

    void frameEnd();

    void shutdown();

private:
    RendererContext context_;
    uint32_t imageIndex = 0;
    uint32_t currentFrame= 0;
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
    VkRenderPass renderPass_;
    VkPipeline graphicPipeline_;
    VkShaderModule vertShader_;
    VkShaderModule fragShader_;
    int MAX_FRMAE_IN_FLIGHTS = 2;
    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkFramebuffer> framebuffers_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> imageFinishedSemaphores_;
    std::vector<VkFence> inFlights_;
    VkCommandPool commandPool_;

};