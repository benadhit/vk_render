#include "Application.h"
#define GLAD_VULKAN_IMPLEMENTATION
#include "vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
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

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainSettigs {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
};

SwapchainSettigs selectOptimalSwapchainSetting(const SwapchainSupportDetails& details)
{
    SwapchainSettigs settings;
    if (details.formats.size() == 1 && details.formats[0].format == VK_FORMAT_UNDEFINED) {
        settings.format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    size_t index =0;
    settings.format = details.formats[0];
    for (auto format : details.formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            settings.format = format;
            break;
        }
    }

    
    settings.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for(auto presentMode : details.presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            settings.presentMode = presentMode;
            break;
        }
    }
    return settings;
}


SwapchainSupportDetails fetchSwapchainSupportDetails(VkPhysicalDevice physicalDevice, 
    VkDevice device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities));
    uint32_t formatCount =0; 
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
    details.formats.resize(formatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,  details.formats.data()));
    uint32_t presetCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presetCount, nullptr));
    details.presentModes.resize(presetCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presetCount, nullptr));
    return details;
}

VkPhysicalDevice getPhysicalDevice(VkInstance instance)
{
    VkPhysicalDevice physicalDevices[16];
    uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

    // TODO: select best physical device;
    return physicalDevices[0];
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

    // create Surface
    VkWin32SurfaceCreateInfoKHR surfaceInfo ={};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.hwnd = glfwGetWin32Window(window);
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.pNext = nullptr;
    vkCreateWin32SurfaceKHR(instance_, &surfaceInfo, nullptr, &surface_);

    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_,
         "vkCreateDebugUtilsMessengerEXT");

    VK_CHECK(fn(instance_, &debugCreateInfo, nullptr, &debugMessenger));



    physicalDevice_ = getPhysicalDevice(instance_);

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &physicalDeviceProperties);

  
    VkQueueFamilyProperties familyProperties[16];
    uint32_t queueFamilyCount = sizeof(familyProperties)/ sizeof(familyProperties[0]);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount,familyProperties);
    
    uint32_t grahicsQueueFamilyIndex = UINT32_MAX;
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
        VkQueueFamilyProperties queueFamily = familyProperties[queueFamilyIndex];
        VkBool32 presentSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, queueFamilyIndex, surface_, &presentSupported);
        if (presentSupported) 
        {
            presentQueueFamilyIndex_ = queueFamilyIndex;
        }

        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
            grahicsQueueFamilyIndex = queueFamilyIndex;
            graphicsQueueFamilyIndex_ = queueFamilyIndex;
            break;
        }
    }

    float prioirties = 1.0f;
    VkDeviceQueueCreateInfo queueInfo ={};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = nullptr;
    queueInfo.pQueuePriorities = &prioirties;
    queueInfo.queueCount = familyProperties[grahicsQueueFamilyIndex].queueCount;
    queueInfo.queueFamilyIndex = grahicsQueueFamilyIndex;
    queueInfo.flags = 0;

    const char* deviceExtension[] = {
        "VK_KHR_swapchain",
    };

    VkPhysicalDeviceFeatures deviceFeatures ={};
    VkDeviceCreateInfo deviceInfo ={};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = sizeof(deviceExtension)/sizeof(deviceExtension[0]);
    deviceInfo.ppEnabledExtensionNames = deviceExtension;
    VK_CHECK(vkCreateDevice(physicalDevice_, &deviceInfo, nullptr, &device_));
    vkGetDeviceQueue(device_, graphicsQueueFamilyIndex_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentQueueFamilyIndex_, 0, &presentQueue_);


    
}

void Application::shutdownVukan()
{
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    surface_ = NULL;
    if (debugMessenger) {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_,
            "vkDestroyDebugUtilsMessengerEXT");
        fn(instance_, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
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