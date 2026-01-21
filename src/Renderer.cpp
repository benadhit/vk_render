#include "Renderer.h"
#include "Vertex.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Renderer::Renderer(const RendererContext& context)
    : context_(context)
{

}

void Renderer::init(const char* vertSpv, const char* fragSpv)
{
    vertShader_ = createShaderModule(context_.device_, vertSpv);
    fragShader_ = createShaderModule(context_.device_, fragSpv);
    std::vector<VkDescriptorSetLayoutBinding> bindings(2);

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount =1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].pImmutableSamplers = nullptr;

    descriptorSetLayout_ = createDescriptorSetLayout(context_.device_, 
        bindings.data(),
        bindings.size()); 
    pipelineLayout_ = createPipelineLayout(context_.device_, &descriptorSetLayout_, 1);
    renderPass_ = createRenderPass(context_.device_);
    graphicPipeline_ = createGrphicsPipeline(context_.device_, 
        pipelineLayout_, 
        renderPass_, 
        vertShader_,
        fragShader_,
        context_.extent_.width,
        context_.extent_.height);

    
    framebuffers_.resize(context_.imageViews_.size());
    for (uint32_t i = 0; i < framebuffers_.size(); ++ i) {
        framebuffers_[i] = createFrambuffer(context_.device_, renderPass_, 
            context_.imageViews_[i], 
            context_.extent_);
    }

    // sync object
    MAX_FRMAE_IN_FLIGHTS = framebuffers_.size();
    commandBuffers_.resize(MAX_FRMAE_IN_FLIGHTS);
    imageAvailableSemaphores_.resize(MAX_FRMAE_IN_FLIGHTS);
    imageFinishedSemaphores_.resize(MAX_FRMAE_IN_FLIGHTS);
    inFlights_.resize(MAX_FRMAE_IN_FLIGHTS);

    for (uint32_t i = 0; i < MAX_FRMAE_IN_FLIGHTS; ++ i) {
        commandBuffers_[i] = createCommandBuffer(context_.device_, context_.commandPool_);
        imageAvailableSemaphores_[i] = createSemaphore(context_.device_);
        imageFinishedSemaphores_[i] = createSemaphore(context_.device_);
        inFlights_[i] = createFence(context_.device_);
    }

    VkDeviceSize vertexSize = vertices.size() * sizeof(Vertex);
    VkDeviceSize indexSize = indices.size()  * sizeof(uint16_t);
    stagingBuffer_ = createBuffer(context_.device_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1024 * 1024 * 24);
    stagingBufferMemory_ = createBufferMemory(
        context_.device_,
        stagingBuffer_,
        context_.memoryProperties_, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(context_.device_, stagingBuffer_, stagingBufferMemory_, 0);
    vkMapMemory(context_.device_, stagingBufferMemory_, 0, 1024 * 1024 * 24, 0, &start_ptr);
    memcpy(start_ptr, vertices.data(), vertexSize);
    createVertexBuffer(vertexSize);
    copyBuffer(context_.device_,
        context_.graphicsQueue_,
        context_.commandPool_, 
        vertexBuffer_,
        stagingBuffer_, 
        vertexSize);

    createIndexBuffer(indices.size()  *sizeof(uint16_t));
    memcpy(start_ptr, indices.data(), indexSize);
    copyBuffer(context_.device_,
        context_.graphicsQueue_,
        context_.commandPool_, 
        indexBuffer_,
        stagingBuffer_, 
        indexSize);
    createUniformBuffers();
    createDescriptorSets();
    loadTexture("../texture.jpg");

}

void Renderer::frameStart()
{
    vkWaitForFences(context_.device_, 1, &inFlights_[currentFrame], VK_TRUE, UINT64_MAX);
    if (inFlights_[currentFrame] != VK_NULL_HANDLE)
    {
        vkResetFences(context_.device_, 1, &inFlights_[currentFrame]);
    }

    vkAcquireNextImageKHR(context_.device_,
        context_.swapchain_,
        UINT64_MAX, 
        imageAvailableSemaphores_[currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);
    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);
    VkCommandBufferBeginInfo beginInfo ={};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkClearValue clearValue = {};
    clearValue.color = {0,0,0,1};

    VkRenderPassBeginInfo passBeginInfo = {};
    passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    passBeginInfo.pNext = nullptr;
    passBeginInfo.renderPass = renderPass_;
    passBeginInfo.clearValueCount =1;
    passBeginInfo.pClearValues = &clearValue;
    passBeginInfo.renderArea.extent = context_.extent_;
    passBeginInfo.renderArea.offset = {0,0};
    passBeginInfo.framebuffer = framebuffers_[imageIndex];
    vkCmdBeginRenderPass(commandBuffer, &passBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

      
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context_.extent_.width);
    viewport.height = static_cast<float>(context_.extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = context_.extent_;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}

void Renderer::render()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
    frameStart();
    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame];
    VkBuffer uniformBuffer = uniformBuffers_[imageIndex];
    VkDeviceMemory uniformBufferMemory = uniformBufferMemorys_[imageIndex];
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffers_[imageIndex];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = textureImageView_;
    imageInfo.sampler = textureSampler_;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> writeDescriptorSet(2);
    writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[0].pNext = nullptr;
    writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet[0].dstBinding = 0;
    writeDescriptorSet[0].dstArrayElement = 0;
    writeDescriptorSet[0].pTexelBufferView = nullptr;
    writeDescriptorSet[0].pImageInfo = nullptr;
    writeDescriptorSet[0].pBufferInfo = &bufferInfo;
    writeDescriptorSet[0].descriptorCount = 1;
    writeDescriptorSet[0].dstSet = descriptorSets_[imageIndex];

    writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[1].pNext = nullptr;
    writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet[1].dstBinding = 1;
    writeDescriptorSet[1].dstArrayElement = 0;
    writeDescriptorSet[1].pTexelBufferView = nullptr;
    writeDescriptorSet[1].pImageInfo = &imageInfo;
    writeDescriptorSet[1].pBufferInfo = nullptr;
    writeDescriptorSet[1].descriptorCount = 1;
    writeDescriptorSet[1].dstSet = descriptorSets_[imageIndex];
    vkUpdateDescriptorSets(context_.device_, writeDescriptorSet.size(), 
    writeDescriptorSet.data(), 0, nullptr);

    UniformBufferObject ubo;
    ubo.model = glm::rotate(glm::mat4(1.0), time * glm::radians(90.f), glm::vec3(0,0,1));
    ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.0f), glm::vec3(0,0,0), glm::vec3(0,0,1));
    ubo.proj =glm::perspective(glm::radians(45.f), 
        context_.extent_.width / (float)context_.extent_.height,
        0.1f, 10.f);
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(context_.device_, uniformBufferMemory, 0, sizeof(UniformBufferObject), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(context_.device_, uniformBufferMemory);
    
    VkBuffer vertexBuffers[] = {vertexBuffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline_);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipelineLayout_, 0, 1, &descriptorSets_[imageIndex], 0, nullptr);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, 
        static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    frameEnd();
}

void Renderer::frameEnd()
{
    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame];
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    //
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame]};
    VkSemaphore signalSemaphores[] = {imageFinishedSemaphores_[currentFrame]};
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext=  nullptr;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    vkQueueSubmit(context_.graphicsQueue_, 1, &submitInfo,  inFlights_[currentFrame]);

    VkPresentInfoKHR presentInfo ={};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext =nullptr;
    presentInfo.pResults = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context_.swapchain_;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    vkQueuePresentKHR(context_.graphicsQueue_, &presentInfo);
    currentFrame = (currentFrame + 1) % MAX_FRMAE_IN_FLIGHTS;
}

void Renderer::shutdown()
{
    vkDeviceWaitIdle(context_.device_);
    vkDestroyBuffer(context_.device_, stagingBuffer_, nullptr);
    vkFreeMemory(context_.device_, stagingBufferMemory_, nullptr);
    
    vkDestroyBuffer(context_.device_, vertexBuffer_, nullptr);
    vkFreeMemory(context_.device_, vertexBufferMemory_, nullptr);

    vkDestroyBuffer(context_.device_, indexBuffer_, nullptr);
    vkFreeMemory(context_.device_, indexBufferMemory_, nullptr);

    vkDestroyImage(context_.device_, textureImage_, nullptr);
    vkDestroyImageView(context_.device_, textureImageView_, nullptr);
    vkFreeMemory(context_.device_, textureImageMemory_, nullptr);

    vkDestroySampler(context_.device_, textureSampler_, nullptr);
    vkDestroyDescriptorSetLayout(context_.device_, descriptorSetLayout_, nullptr);

    vkFreeDescriptorSets(context_.device_, context_.descriptorPool_, descriptorSets_.size(),
        descriptorSets_.data());
    
    vkDestroyDescriptorPool(context_.device_, context_.descriptorPool_, nullptr);
    for (auto& uniformBuffer : uniformBuffers_) {
        vkDestroyBuffer(context_.device_, uniformBuffer, nullptr);
    }
    for (auto& uniformBufferMemory : uniformBufferMemorys_) {
        vkFreeMemory(context_.device_, uniformBufferMemory, nullptr);
    }


    for (uint32_t i = 0 ; i < MAX_FRMAE_IN_FLIGHTS; ++i) {
        vkDestroySemaphore(context_.device_, imageAvailableSemaphores_[i], nullptr);
        vkDestroySemaphore(context_.device_, imageFinishedSemaphores_[i], nullptr);
        vkDestroyFence(context_.device_, inFlights_[i], nullptr);
    }
    imageAvailableSemaphores_.clear();
    imageFinishedSemaphores_.clear();
    inFlights_.clear();

    vkDestroyCommandPool(context_.device_, context_.commandPool_, nullptr);
    for (auto & framebuffer: framebuffers_) {
        vkDestroyFramebuffer(context_.device_, framebuffer, nullptr);
    }
    framebuffers_.clear();
    vkDestroyShaderModule(context_.device_, vertShader_, nullptr);
    vkDestroyShaderModule(context_.device_, fragShader_, nullptr);
    vkDestroyPipelineLayout(context_.device_, pipelineLayout_, nullptr);
    vkDestroyRenderPass(context_.device_, renderPass_, nullptr);
    vkDestroyPipeline(context_.device_, graphicPipeline_, nullptr);
}

void Renderer::createVertexBuffer(size_t vertexSize)
{
    vertexBuffer_ = createBuffer(context_.device_, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        vertexSize);
    vertexBufferMemory_ = createBufferMemory(context_.device_, 
        vertexBuffer_, 
        context_.memoryProperties_, 
        VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    vkBindBufferMemory(context_.device_, vertexBuffer_, vertexBufferMemory_, 0);
}

void Renderer::createIndexBuffer(size_t indexSize)
{
    uint32_t indiceSize = sizeof(uint32_t) * indices.size();
    indexBuffer_ = createBuffer(context_.device_, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        indexSize);
    indexBufferMemory_ = createBufferMemory(context_.device_, 
        indexBuffer_, 
        context_.memoryProperties_, 
        VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    
    vkBindBufferMemory(context_.device_, indexBuffer_, indexBufferMemory_, 0);
}

void Renderer::createUniformBuffers()
{
    uint32_t uniformSize = sizeof(UniformBufferObject);
    uniformBufferMemorys_.resize(framebuffers_.size());
    uniformBuffers_.resize(framebuffers_.size());
    for (uint32_t i = 0; i < framebuffers_.size(); ++i) 
    {
        uniformBuffers_[i] = createBuffer(context_.device_, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            uniformSize);
        uniformBufferMemorys_[i] = createBufferMemory(context_.device_, 
            uniformBuffers_[i], 
            context_.memoryProperties_, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
        vkBindBufferMemory(context_.device_, uniformBuffers_[i], uniformBufferMemorys_[i], 0);
    }
}

void Renderer::createDescriptorSets()
{
    descriptorSets_.resize(context_.imageViews_.size());
    for (uint32_t i = 0; i < context_.imageViews_.size(); ++i)
    {
        descriptorSets_[i] = createDescriptorSet(context_.device_,
            context_.descriptorPool_,
            &descriptorSetLayout_,
            1);
    }
}

void Renderer::loadTexture(const char* path)
{
    if (path == nullptr || strcmp(path, "") == 0)
        return;
    int texWidth;
    int texHeight;
    int texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        return;
    }
    textureImage_ = createImage2D(context_.device_, 
        VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        texWidth,
        texHeight,
        1,
        1);
    textureImageMemory_ = createImageMemory(context_.device_,
        textureImage_,
        context_.memoryProperties_,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindImageMemory(context_.device_,
        textureImage_,
        textureImageMemory_,
        0);
    memcpy(start_ptr, pixels, texWidth * texHeight * 4);
    
    stbi_image_free(pixels);
    pixels = nullptr;


    // 将纹理layout 转换成 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    // 以便GPU传输
    transitionImageLayout(context_.device_,
        context_.graphicsQueue_, 
        context_.commandPool_,
        textureImage_,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(context_.device_,
        context_.graphicsQueue_, 
        context_.commandPool_,
        textureImage_,
        stagingBuffer_,
        texWidth,
        texHeight);
    // 
    transitionImageLayout(context_.device_,
        context_.graphicsQueue_, 
        context_.commandPool_,
        textureImage_,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    textureImageView_ = createImageView2D(context_.device_,
        textureImage_,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8G8B8A8_UNORM);
    textureSampler_ = createSampler(context_.device_);
}