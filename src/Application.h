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
private:
    GLFWwindow* window;
    VkInstance instance_{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    bool validationEnable = true;
    VkDebugUtilsMessengerEXT debugMessenger;
};
#endif