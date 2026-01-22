#pragma once
#include "VulkanUtils.h"

struct RendererContext {
    VkExtent2D extent_;
    VkFormat format_;
    uint32_t graphicsQueueFamilyIndex;
    VkQueue graphicsQueue_;
    VkPhysicalDevice physicalDevice_;
    VkCommandPool commandPool_;
    VkDescriptorPool descriptorPool_;
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
    void createUniformBuffers();
    void createDescriptorSets();
    void loadTexture(const char* path);
    void createDepthTexture(uint32_t width, uint32_t height);
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

    std::vector<VkFramebuffer> framebuffers_;
    // sync object
    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> imageFinishedSemaphores_;
    std::vector<VkFence> inFlights_;

    // DescriptorSet
    VkDescriptorSetLayout descriptorSetLayout_;

    // Uniform Buffers
    std::vector<VkBuffer> uniformBuffers_;
    std::vector<VkDeviceMemory> uniformBufferMemorys_;
    std::vector<VkDescriptorSet> descriptorSets_;

    // Images
    VkImage textureImage_{VK_NULL_HANDLE};
    VkDeviceMemory textureImageMemory_{VK_NULL_HANDLE};

    VkImageView textureImageView_{VK_NULL_HANDLE};
    VkSampler textureSampler_{VK_NULL_HANDLE};
    VkFormat colorFormat_;
    // Depth Image
    VkFormat depthFormat_;
    VkImage depthImage_{VK_NULL_HANDLE};
    VkDeviceMemory depthImageMemory_{VK_NULL_HANDLE};

    VkImageView depthImageView_{VK_NULL_HANDLE};

    // Upload Buffer
    VkBuffer stagingBuffer_{VK_NULL_HANDLE}; 
    VkDeviceMemory stagingBufferMemory_{VK_NULL_HANDLE};
    void* start_ptr; // 开始地址
};