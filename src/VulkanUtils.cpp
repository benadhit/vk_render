#include "VulkanUtils.h"
#include "vulkan.h"
#include <iostream>
#include <algorithm>
#include "IoUtils.h"
#include "Vertex.h"

bool checkRequireExtensions(const std::vector<const char*>& requiredExtensions)
{
    VkExtensionProperties extensions[128];
    uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    std::cout << "Available Extensions:\n"; 
    for (uint32_t idx = 0; idx < requiredExtensions.size(); ++idx)
    {
        bool supported = false;
        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            if (strcmp(requiredExtensions[idx], extensions[i].extensionName) == 0) {
                supported = true;
                break;
            }
        }
        if (!supported)
        {
            return false;
        }

        // std::cout << "\t" << extensions[idx].extensionName
        //         << "\t" << extensions[idx].specVersion 
        //         << "\t" << (supported ? "[GLFW Extension]" : "") << "\n";
    }
    return true;
}

bool checkRequiredLayerExtension(const std::vector<const char*>& requiredLayers)
{
    VkLayerProperties layers[128];
    uint32_t layerCount = sizeof(layers) / sizeof(layers[0]);
    vkEnumerateInstanceLayerProperties(&layerCount, layers);
    for (size_t ridx = 0; ridx < requiredLayers.size(); ++ridx )
    {
        bool supported = false;
        for (uint32_t idx = 0; idx < layerCount; ++idx)
        {
            if (strcmp(requiredLayers[ridx], layers[idx].layerName) == 0) {
                supported = true;
                break;
            }
        }
        if (!supported)
        {
            return false;
        }
        // std::cout << "\t" << layers[ridx].layerName
        // << "\t" << layer[ridx].specVersion 
        // << "\t" << (supported ? "[GLFW Extension]" : "") << "\n";
    }
    return true;
}

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
        if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            settings.presentMode = presentMode;
        }
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            settings.presentMode = presentMode;
            break;
        }
    }

    if (details.capabilities.currentExtent.width != UINT32_MAX) {
        settings.extent = details.capabilities.currentExtent;
    }
    else {
        settings.extent.width = clamp(settings.extent.width, 
        details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
        settings.extent.width = clamp(settings.extent.height, 
        details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
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
    uint32_t physicalDeviceCount  =0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    std::vector<VkPhysicalDevice>  physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
    // TODO: select best physical device;
    return physicalDevices[0];
}

VkDevice createDevice(VkPhysicalDevice physicalDevice, 
    VkSurfaceKHR surface,
    uint32_t& graphicsQueueFamilyIndex,
    uint32_t& presentQueueFamilyIndex)
{
    VkQueueFamilyProperties familyProperties[16];
    uint32_t queueFamilyCount = sizeof(familyProperties)/ sizeof(familyProperties[0]);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, 
        &queueFamilyCount,familyProperties);
    
    uint32_t grahicsQueueFamilyIndex = UINT32_MAX;
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
        VkQueueFamilyProperties queueFamily = familyProperties[queueFamilyIndex];
        VkBool32 presentSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupported);
        if (presentSupported) 
        {
            presentQueueFamilyIndex = queueFamilyIndex;
        }

        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
            grahicsQueueFamilyIndex = queueFamilyIndex;
            graphicsQueueFamilyIndex = queueFamilyIndex;
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

    VkPhysicalDeviceFeatures features={};
    features.samplerAnisotropy = VK_TRUE;
    VkPhysicalDeviceFeatures deviceFeatures ={};
    VkDeviceCreateInfo deviceInfo ={};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = sizeof(deviceExtension)/sizeof(deviceExtension[0]);
    deviceInfo.ppEnabledExtensionNames = deviceExtension;
    deviceInfo.pEnabledFeatures = &features;
    VkDevice device{VK_NULL_HANDLE};
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
    return device ;
}

VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice,
    VkDevice device, VkSurfaceKHR surface, 
    const SwapchainSupportDetails& details,
    const SwapchainSettigs& setting,
    uint32_t graphicsQueueFamilyIndex, 
    uint32_t presentQueueFamilyIndex)
{
    uint32_t imageCount = details.capabilities.minImageCount;

    VkSwapchainCreateInfoKHR swapchainInfo ={};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.flags = 0;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageExtent = setting.extent;
    swapchainInfo.presentMode = setting.presentMode;
    swapchainInfo.imageFormat = setting.format.format;
    swapchainInfo.imageColorSpace = setting.format.colorSpace;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.pNext = nullptr;
    swapchainInfo.surface = surface;
    swapchainInfo.imageArrayLayers = 1;

    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
        uint32_t queueFamilyIndices[] = {graphicsQueueFamilyIndex, presentQueueFamilyIndex} ;
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
        swapchainInfo.queueFamilyIndexCount = sizeof(queueFamilyIndices)/ sizeof(queueFamilyIndices[0]);
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    else {
        swapchainInfo.pQueueFamilyIndices = nullptr;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainInfo.preTransform = details.capabilities.currentTransform;
    swapchainInfo.clipped = VK_TRUE;
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));
    return swapchain;
}

VkImageView createImageView(VkDevice device, 
    VkImage image,
    VkImageViewType viewType,
    VkImageAspectFlags aspectFlag, 
    VkFormat format)
{
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.viewType = viewType;
    info.image = image;
    info.format = format;
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = aspectFlag;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.levelCount = 1;
    VkImageView imageView{VK_NULL_HANDLE};
    VK_CHECK(vkCreateImageView(device, &info, nullptr, &imageView));
    return imageView;
}


VkImageView createImageView2D(VkDevice device, VkImage image, 
    VkImageAspectFlags aspectFlag, 
    VkFormat format)
{
    return createImageView(device, 
        image, 
        VK_IMAGE_VIEW_TYPE_2D, 
        aspectFlag,
        format);
}

VkSampler createSampler(VkDevice device)
{
    VkSamplerCreateInfo samplerInfo ={};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.f;
    samplerInfo.minLod = 0.f;
    samplerInfo.maxLod = 0.f;

    VkSampler sampler {VK_NULL_HANDLE};
    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
    return sampler;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queueFamilyIndex;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VK_CHECK(vkCreateCommandPool(device, &info, nullptr, &commandPool));
    return commandPool;
}

VkSemaphore createSemaphore(VkDevice device)
{
    VkSemaphoreCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    VkSemaphore semaphore{VK_NULL_HANDLE};
    VK_CHECK(vkCreateSemaphore(device, &info, nullptr, &semaphore));
    return semaphore;
}

VkPipelineLayout createPipelineLayout(VkDevice device,
    VkDescriptorSetLayout* descriptorSetLayout, 
    uint32_t descriptorSetLayoutSize)
{
    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.setLayoutCount = descriptorSetLayoutSize;
    layoutInfo.pSetLayouts = descriptorSetLayout;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges= nullptr;
    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkShaderModule createShaderModule(VkDevice device, const char* spvPath)
{
    std::vector<char> spv = readFile(spvPath);
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.flags = 0;
    info.pCode = reinterpret_cast<uint32_t*>(spv.data());
    info.codeSize = static_cast<uint32_t>(spv.size());
    VkShaderModule shaderModule {VK_NULL_HANDLE};
    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shaderModule));
    return shaderModule;
}

VkRenderPass createRenderPass(VkDevice device)
{
    VkAttachmentDescription colorAttachments[1] = {};
    colorAttachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    colorAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachments[0].flags = 0;
    colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 
    VkAttachmentReference colorAttachmentReferences[1];
    colorAttachmentReferences[0].attachment = 0;
    colorAttachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.colorAttachmentCount = ARRAY_SIZE(colorAttachmentReferences);
    subpassDescription.pColorAttachments = colorAttachmentReferences;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // 呈现
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    

    VkRenderPassCreateInfo passInfo = {};
    passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    passInfo.flags = 0;
    passInfo.pNext= nullptr;
    passInfo.attachmentCount = ARRAY_SIZE(colorAttachments);
    passInfo.pAttachments = colorAttachments;
    passInfo.subpassCount = 1;
    passInfo.pSubpasses = &subpassDescription;
    passInfo.dependencyCount =1;
    passInfo.pDependencies= &dependency;

    VkRenderPass renderPass{VK_NULL_HANDLE};
    VK_CHECK(vkCreateRenderPass(device, &passInfo, nullptr, &renderPass));
    return renderPass;
}


VkPipeline createGrphicsPipeline(VkDevice device, 
    VkPipelineLayout pipelineLayout,
    VkRenderPass renderPass,
    VkShaderModule vertShaderModule, 
    VkShaderModule fragShaderModule,
    uint32_t width,
    uint32_t height)
{
    VkPipelineShaderStageCreateInfo shaderStages[2]{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].flags = 0;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].flags = 0;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";
    shaderStages[1].pSpecializationInfo = nullptr;

    VkVertexInputBindingDescription bindingDescription = Vertex::getVertexBindingDescription();
    auto attributes = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.flags = 0;
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    // 
    VkPipelineInputAssemblyStateCreateInfo inputAssembly ={};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkDynamicState dynamicStates [] ={
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)width;
    viewport.height = (float)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {width, height};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.flags = 0;
    dynamicStateInfo.pNext = nullptr;
    dynamicStateInfo.pDynamicStates = dynamicStates;
    dynamicStateInfo.dynamicStateCount = ARRAY_SIZE(dynamicStates);

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    VkPipelineRasterizationStateCreateInfo rasterizationState ={};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.pNext = nullptr;
    rasterizationState.flags = 0;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE ;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.f;
    rasterizationState.depthBiasConstantFactor = 0.f;
    rasterizationState.depthBiasClamp = 0.f;
    rasterizationState.depthBiasSlopeFactor = 0.f;

    VkPipelineMultisampleStateCreateInfo multisamplingInfo ={};
    multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingInfo.pNext = nullptr;
    multisamplingInfo.sampleShadingEnable = VK_FALSE;
    multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingInfo.minSampleShading = 1.f;
    multisamplingInfo.pSampleMask = nullptr;
    multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo ={};

    //
    VkPipelineColorBlendAttachmentState colorBlendAttachment= {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendInfo ={};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.flags = 0;
    colorBlendInfo.pNext = 0;
    colorBlendInfo.blendConstants[0] = 0.f;
    colorBlendInfo.blendConstants[1] = 0.f;
    colorBlendInfo.blendConstants[2] = 0.f;
    colorBlendInfo.blendConstants[3] = 0.f;    
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount =1;
    colorBlendInfo.pAttachments  =&colorBlendAttachment;

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.pNext =  nullptr;
    graphicsPipelineInfo.flags = 0;
    graphicsPipelineInfo.stageCount= ARRAY_SIZE(shaderStages);
    graphicsPipelineInfo.pStages = shaderStages;
    graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineInfo.pViewportState = &viewportState;;
    graphicsPipelineInfo.pRasterizationState = &rasterizationState;
    graphicsPipelineInfo.pMultisampleState = &multisamplingInfo;
    graphicsPipelineInfo.pColorBlendState = &colorBlendInfo;
    graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
    graphicsPipelineInfo.pDepthStencilState = nullptr;
    graphicsPipelineInfo.layout = pipelineLayout;
    graphicsPipelineInfo.renderPass = renderPass;
    graphicsPipelineInfo.subpass = 0;
    graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineInfo.basePipelineIndex = -1;

    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
    VK_CHECK(vkCreateGraphicsPipelines(device, 
        VK_NULL_HANDLE, 
        1, 
        &graphicsPipelineInfo, 
        nullptr, 
        &graphicsPipeline));

    return graphicsPipeline;
}

VkFramebuffer createFrambuffer(VkDevice device,
    VkRenderPass renderPass,
    VkImageView imageView,
    VkExtent2D extent)
{
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView; 
    framebufferInfo.layers = 1;
    framebufferInfo.renderPass  = renderPass;
    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    VkFramebuffer framebuffer {VK_NULL_HANDLE};
    VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer));
    return framebuffer;
}

VkCommandBuffer createCommandBuffer(VkDevice device, 
    VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo commandBufferInfo ={};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferInfo.pNext =  nullptr;
    commandBufferInfo.commandBufferCount =1;
    commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandPool = commandPool;
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferInfo, &commandBuffer));
    return commandBuffer;
}

VkFence createFence(VkDevice device)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkFence fence{VK_NULL_HANDLE};
    VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &fence));
    return fence;
}

VkBuffer createBuffer(VkDevice device, 
    VkBufferUsageFlags usage,
    size_t size)
{
    VkBufferCreateInfo bufferInfo ={};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.pQueueFamilyIndices = nullptr;
    bufferInfo.queueFamilyIndexCount = 0;
    VkBuffer buffer{VK_NULL_HANDLE};
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));
    return buffer;
}

VkDeviceMemory createBufferMemory(VkDevice device, 
    VkBuffer buffer,
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkMemoryRequirements memoryRequirements ={};
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;   
    allocInfo.memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, 
        memoryPropertyFlags);
    allocInfo.allocationSize = memoryRequirements.size;
    VkDeviceMemory deviceMemory {VK_NULL_HANDLE};
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory));
    return deviceMemory;
}

VkDeviceMemory createImageMemory(VkDevice device,
    VkImage image, 
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkMemoryRequirements memoryRequirements ={};
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;   
    allocInfo.memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, 
        memoryPropertyFlags);
    allocInfo.allocationSize = memoryRequirements.size;
    VkDeviceMemory deviceMemory {VK_NULL_HANDLE};
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory));
    return deviceMemory;
}

void createBufferWithMemory(VkDevice device,
    VkBuffer* buffer,
    VkDeviceSize bufferSize,
    VkBufferUsageFlags bufferUsage,
    VkDeviceMemory *bufferMemory, 
    VkPhysicalDeviceMemoryProperties memoryProperties,
    VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkBufferCreateInfo bufferInfo ={};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = bufferUsage;
    bufferInfo.pQueueFamilyIndices = nullptr;
    bufferInfo.queueFamilyIndexCount = 0;
    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, buffer));

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;   
    allocInfo.memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, 
        memoryPropertyFlags);
    allocInfo.allocationSize = memoryRequirements.size;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory));
    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

VkDescriptorSet createDescriptorSet(VkDevice device,
    VkDescriptorPool descriptorPool,
    VkDescriptorSetLayout *descriptorSetLayout,
    uint32_t descriptorSetLayoutSize)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.pNext = nullptr;
    descriptorSetAllocInfo.pSetLayouts = descriptorSetLayout;
    descriptorSetAllocInfo.descriptorSetCount = descriptorSetLayoutSize;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSet);
    return descriptorSet;
}

uint32_t findMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties,
    uint32_t typeFilter, 
    VkMemoryPropertyFlags properties)
{
    for (uint32_t i =0; i < memoryProperties.memoryTypeCount; i++) {
        uint32_t memoryTypeProperties = memoryProperties.memoryTypes[i].propertyFlags;
        if (typeFilter & (1 << i) && (memoryTypeProperties & properties) == properties) {
            return i;
        }
    } 
    return UINT32_MAX;
}

void copyBuffer(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkBuffer dstBuffer,
    VkBuffer srcBuffer,
    VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommandBuffer(device, commandPool);
    VkBufferCopy copyRegion;
    copyRegion.size = size;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommandBuffer(device, queue, commandPool,commandBuffer);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkCommandBuffer beginSingleTimeCommandBuffer(VkDevice device, 
    VkCommandPool commandPool)
{
    VkCommandBuffer commandBuffer = createCommandBuffer(device, commandPool);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void endSingleTimeCommandBuffer(
    VkDevice device,
    VkQueue commandQueue,
    VkCommandPool commandPool,
    VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;
    vkQueueSubmit(commandQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkDeviceWaitIdle(device);
}

VkDescriptorPool createDescriptorPool(VkDevice device)
{
    VkDescriptorPoolSize poolSize[2];
    poolSize[0].descriptorCount = 128;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[1].descriptorCount = 128;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ;
    poolInfo.maxSets = 512;
    poolInfo.poolSizeCount =2;
    poolInfo.pPoolSizes = poolSize;
    VkDescriptorPool pool{VK_NULL_HANDLE};
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool));
    return pool;
}

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device,
    VkDescriptorSetLayoutBinding* bindings, uint32_t bindingSize)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetInfo ={};
    descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetInfo.flags = 0;
    descriptorSetInfo.pNext = nullptr;
    descriptorSetInfo.pBindings = bindings;
    descriptorSetInfo.bindingCount = bindingSize;
    VkDescriptorSetLayout descriptorSetLayout ={VK_NULL_HANDLE};
    VK_CHECK(vkCreateDescriptorSetLayout(device, &descriptorSetInfo, nullptr, &descriptorSetLayout));
    return descriptorSetLayout;
}

static VkImage createImage(
    VkDevice device,
    VkFormat format,
    VkImageUsageFlags usage,
    VkImageType imageType,
    VkSampleCountFlagBits samples,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t mipLevels,
    uint32_t layers)
{
    VkImageCreateInfo imageInfo ={};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.flags = 0;
    imageInfo.format = format;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layers;
    imageInfo.samples = samples;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImage image = {VK_NULL_HANDLE};
    VK_CHECK(vkCreateImage(device, &imageInfo, nullptr, &image));
    return image;
}

VkImage createImage2D(VkDevice device,
    VkFormat format,
    VkImageUsageFlags usage,
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    uint32_t layers)
{
    return createImage(device, format, usage, VK_IMAGE_TYPE_2D,
        VK_SAMPLE_COUNT_1_BIT, width, height, 1, mipLevels, layers);
    
}

VkImage createImage3D(VkDevice device,
    VkFormat format,
    VkImageUsageFlags usage,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t mipLevels,
    uint32_t layers)
{
    return createImage(device, format, usage, VK_IMAGE_TYPE_3D,
        VK_SAMPLE_COUNT_1_BIT, width, height, depth, mipLevels, layers);
}

void transitionImageLayout(VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool,
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommandBuffer(device, commandPool);

    VkImageMemoryBarrier imageBarrier ={};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.pNext = nullptr;
    imageBarrier.oldLayout = oldLayout;
    imageBarrier.newLayout = newLayout;

    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    imageBarrier.image= image;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageBarrier.srcAccessMask = 0;
        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&  newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, 
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1,  &imageBarrier);

    endSingleTimeCommandBuffer(device,queue, commandPool, commandBuffer);
}

void copyBufferToImage(VkDevice device, 
    VkQueue queue,
    VkCommandPool commandPool,
    VkImage image, 
    VkBuffer buffer,
    uint32_t width,
    uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommandBuffer(device, commandPool);

    VkBufferImageCopy region = {};
    region.bufferOffset =  0;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;

    region.imageOffset ={0,0,0};
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount= 1;
    region.imageSubresource.mipLevel = 0;

    vkCmdCopyBufferToImage(commandBuffer, 
        buffer, 
        image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1 ,
        &region);

    endSingleTimeCommandBuffer(device, queue, commandPool, commandBuffer);
}