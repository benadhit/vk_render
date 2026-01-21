#pragma once 
#include "VulkanUtils.h"
#include <array>
#include <glm/glm.hpp>

struct UniformBufferObject{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    // TODO:
    // glm::mat4 mvp;
};

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 uv;
    
    static VkVertexInputBindingDescription getVertexBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription ={};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);
        
        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescription()
    {
        std::vector<VkVertexInputAttributeDescription> attributes(3);
        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format= VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, position);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format= VK_FORMAT_R32G32B32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, color);

        attributes[2].binding = 0;
        attributes[2].location = 2;
        attributes[2].format= VK_FORMAT_R32G32_SFLOAT;
        attributes[2].offset = offsetof(Vertex, uv);
        return attributes;
    }
};

static const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.f, 0.f}},
    {{0.5f, -0.5f}, {1.0f,  1.0f, 0.0f}, {0.f, 0.f}},
    {{0.5f, 0.5f},  {0.0f, 0.0f,  1.0f}, {0.f, 1.f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.f , 1.f}}
};

static const std::vector<uint16_t> indices = {
    0,1,2,
    2,3,0
};