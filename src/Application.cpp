#include "Application.h"
#define GLAD_VULKAN_IMPLEMENTATION
#include "vulkan.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Renderer.h"

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
    initRenderer();
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

void Application::initRenderer()
{
    RendererContext context;
    context.device_ = device_;
    context.extent_ = extent_;
    context.format_ = format_;
    context.graphicsQueueFamilyIndex = graphicsQueueFamilyIndex_;
    context.imageViews_ = swapchainImageViews_;
    context.swapchain_ = swapchain_;
    context.commandPool_ = createCommandPool(device_, graphicsQueueFamilyIndex_);
    context.graphicsQueue_ = graphicsQueue_;
    
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &context.memoryProperties_);
    renderer_ = new Renderer(context);
    renderer_->init("../vert.spv", "../frag.spv");
}

void Application::shutdownWindow()
{
    glfwDestroyWindow(window);
}

void Application::render()
{
    renderer_->render();
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
    instanceInfo.pNext = &debugCreateInfo;
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

    physicalDevice_ = getPhysicalDevice(instance_);

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &physicalDeviceProperties);

    device_ = createDevice(physicalDevice_, surface_, graphicsQueueFamilyIndex_, presentQueueFamilyIndex_);

    vkGetDeviceQueue(device_, graphicsQueueFamilyIndex_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentQueueFamilyIndex_, 0, &presentQueue_);
    gladLoaderLoadVulkan(instance_, physicalDevice_, device_);

    SwapchainSupportDetails details = fetchSwapchainSupportDetails(physicalDevice_, device_, surface_);
    SwapchainSettigs setting = selectOptimalSwapchainSetting(details);
    extent_ = setting.extent;
    format_ = setting.format.format;
    swapchain_ = createSwapchain(physicalDevice_, 
        device_, 
        surface_, 
        details, 
        setting, 
        graphicsQueueFamilyIndex_, 
        presentQueueFamilyIndex_);
    uint32_t swapchianImageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(device_, swapchain_, &swapchianImageCount, nullptr));
    swapchainImages_.resize(swapchianImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(device_, swapchain_, &swapchianImageCount, swapchainImages_.data()));
    swapchainImageViews_.resize(swapchianImageCount);
    for (size_t i =0; i < swapchainImageViews_.size(); ++i) 
    {
        swapchainImageViews_[i] = createImageView(device_, 
            swapchainImages_[i], 
            setting.format.format);
    }
}

void Application::shutdownVukan()
{
    for (auto& imageView: swapchainImageViews_) {
        vkDestroyImageView(device_, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    surface_ = NULL;
    vkDestroyInstance(instance_, nullptr);
}

void Application::mainLoop()
{
    if (!window)
        return;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render();
    }
}