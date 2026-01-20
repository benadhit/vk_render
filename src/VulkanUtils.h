#ifndef HAMON_VULKAN_UTILS_H__
#define HAMON_VULKAN_UTILS_H__
#include <stdint.h>
#include <vector>
#include <assert.h>
#include <GLFW/glfw3.h>
#include "vulkan.h"
#define VK_CHECK(call)                  \
    do{                                 \
        VkResult result = call;         \
        assert(result == VK_SUCCESS);   \
    }while(0)


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])
#endif

template<class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return clamp(v, lo, hi, std::less<T>{});
}

template<class T, class Compare>
constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
{
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainSettigs {
    VkExtent2D extent;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
};

bool checkRequireExtensions(const std::vector<const char*>& requiredExtensions);
bool checkRequiredLayerExtension(const std::vector<const char*>& requiredLayers);

SwapchainSupportDetails fetchSwapchainSupportDetails(VkPhysicalDevice physicalDevice, 
    VkDevice device, 
    VkSurfaceKHR surface);
SwapchainSettigs selectOptimalSwapchainSetting(const SwapchainSupportDetails& details);
VkPhysicalDevice getPhysicalDevice(VkInstance instance);
VkDevice createDevice(VkPhysicalDevice physicalDevice, 
    VkSurfaceKHR surface,
    uint32_t& graphicsQueueFamilyIndex,
    uint32_t& presentQueueFamilyIndex);

VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice,
    VkDevice device, VkSurfaceKHR surface, 
    const SwapchainSupportDetails& detatils,
    const SwapchainSettigs& settings,
    uint32_t graphicsQueueFamilyIndex, 
    uint32_t presentQueueFamilyIndex);

VkImageView createImageView(VkDevice device, 
    VkImage image, 
    VkFormat format);

VkCommandPool createCommandPool(VkDevice device, uint32_t queueFamilyIndex);

VkSemaphore createSemaphore(VkDevice device);

VkShaderModule createShaderModule(VkDevice device, const char* spvPath);

VkPipelineLayout createPipelineLayout(VkDevice device);

VkRenderPass createRenderPass(VkDevice device);

VkFramebuffer createFrambuffer(VkDevice device,
    VkRenderPass renderPass,
    VkImageView imageView,
    VkExtent2D extent);

VkPipeline createGrphicsPipeline(VkDevice device, 
    VkPipelineLayout pipelineLayout,
    VkRenderPass renderPass,
    VkShaderModule vertShaderModule, 
    VkShaderModule fragShaderModule,
    uint32_t width,
    uint32_t height);
VkCommandBuffer createCommandBuffer(VkDevice device, 
    VkCommandPool commandPool);

VkFence createFence(VkDevice device);

VkBuffer createBuffer(VkDevice device, 
    VkBufferUsageFlags usage, 
    size_t size);

VkDeviceMemory createBufferMemory(VkDevice device,
    VkBuffer buffer, 
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkMemoryPropertyFlags memoryPropertyFlags);

void createBufferWithMemory(VkDevice device,
    VkBuffer* buffer,
    VkDeviceSize bufferSize,
    VkBufferUsageFlags bufferUsage,
    VkDeviceMemory *bufferMemory, 
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkMemoryPropertyFlags memoryPropertyFlags);

uint32_t findMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties,
    uint32_t typeFilter, VkMemoryPropertyFlags properties);

void copyBuffer(
    VkDevice device,
    VkQueue queue, 
    VkCommandBuffer commandBuffer,
    VkBuffer dstBuffer,
    VkBuffer srcBuffer,
    VkDeviceSize size);
#endif