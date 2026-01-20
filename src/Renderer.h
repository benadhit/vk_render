#pragma once
#include "VulkanUtils.h"

struct RendererContext {
    VkExtent2D extent_;
    VkFormat format_;
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue_;
    VkPhysicalDevice physicalDevice_;
    VkCommandPool commandPool_;
    VkDevice device_{VK_NULL_HANDLE};
    VkSwapchainKHR  swapchain_;
    VkPhysicalDeviceMemoryProperties memoryProperties_;
    std::vector<VkImageView>   imageViews_;
};

struct FrameData {

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
    void createVertexBuffer(size_t vertexSize);
    void createIndexBuffer(size_t indexSize);
private:
    RendererContext context_;
    VkBuffer vertexBuffer_{VK_NULL_HANDLE};
    VkDeviceMemory vertexBufferMemory_{VK_NULL_HANDLE};
    VkBuffer indexBuffer_{VK_NULL_HANDLE};
    VkDeviceMemory indexBufferMemory_{VK_NULL_HANDLE};
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
};