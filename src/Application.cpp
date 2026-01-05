#include "Application.h"
#define GLAD_VULKAN_IMPLEMENTATION
#include "vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include "VulkanUtils.h"
#include <vector>
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    shutdownVukan();
}

void Application::initWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    const int width = 1280;
    const int heght = 720;
    window = glfwCreateWindow(width, heght, "Vulkan", nullptr, nullptr);
}

void Application::shutdownWindow()
{
    glfwDestroyWindow(window);
}


void Application::initVulkan()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> rquiredExtensions;
    std::copy(glfwExtensions, glfwExtensions+ glfwExtensionCount, std::back_inserter(rquiredExtensions));
    if (validationEnable) {
        rquiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!checkRequireExtensions(rquiredExtensions))
    {
        assert(0);
        return;
    }

    std::vector<const char*> reuqiredLayers;
    if (validationEnable) {
        reuqiredLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    if (!checkRequiredLayerExtension(reuqiredLayers)) {
        return;
    }
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pEngineName = nullptr;
    appInfo.applicationVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.pApplicationName = "PBR SandBox";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo ={};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;
    debugCreateInfo.pUserData = nullptr;   
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = rquiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = rquiredExtensions.data();
    instanceInfo.ppEnabledLayerNames = reuqiredLayers.data();
    instanceInfo.enabledLayerCount = reuqiredLayers.size();
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance_));

    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_,
         "vkCreateDebugUtilsMessengerEXT");

    VK_CHECK(fn(instance_, &debugCreateInfo, nullptr, &debugMessenger));

}

void Application::shutdownVukan()
{
    if (debugMessenger) {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_,
            "vkDestroyDebugUtilsMessengerEXT");
        fn(instance_, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance_, nullptr);
}

void Application::mainLoop()
{
    if (!window)
        return;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}