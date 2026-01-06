#ifndef HAMON_APPLICATION_H__
#define HAMON_APPLICATION_H__

#include <GLFW/glfw3.h>
#include "vulkan.h"
class Application {
public:
    void run();
private:
    void initWindow();
    void shutdownWindow();
    void initVulkan();
    void shutdownVukan();
    
    void mainLoop();

    void createSwapchain(uint32_t width, uint32_t height);
private:
    GLFWwindow* window;
    VkInstance instance_{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
    VkQueue  graphicsQueue_{VK_NULL_HANDLE};
    VkQueue  presentQueue_{VK_NULL_HANDLE};
    uint32_t graphicsQueueFamilyIndex_{UINT32_MAX};
    uint32_t presentQueueFamilyIndex_{UINT32_MAX};
    VkSurfaceKHR    surface_{VK_NULL_HANDLE};
    VkSwapchainKHR  swapchain{VK_NULL_HANDLE};
    bool validationEnable = true;
    VkDebugUtilsMessengerEXT debugMessenger;
};
#endif