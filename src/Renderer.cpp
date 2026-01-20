#include "Renderer.h"
#include "Vertex.h"
Renderer::Renderer(const RendererContext& context)
    : context_(context)
{

}

void Renderer::init(const char* vertSpv, const char* fragSpv)
{
    vertShader_ = createShaderModule(context_.device_, vertSpv);
    fragShader_ = createShaderModule(context_.device_, fragSpv);
    pipelineLayout_ = createPipelineLayout(context_.device_);
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

    VkBuffer stagingBuffer = createBuffer(context_.device_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 1024 * 1024 * 24);

    VkDeviceSize vertexSize = vertices.size() * sizeof(Vertex);
    VkDeviceSize indexSize = indices.size()  * sizeof(uint16_t);

    VkCommandBuffer stagingCommandBuffer = createCommandBuffer(context_.device_, 
        context_.commandPool_);

    VkDeviceMemory stagingBufferMemory = createBufferMemory(
        context_.device_,
        stagingBuffer,
        context_.memoryProperties_, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkBindBufferMemory(context_.device_, stagingBuffer, stagingBufferMemory, 0);
    void * data = nullptr;
    vkMapMemory(context_.device_, stagingBufferMemory, 0, 1024 * 1024 * 24, 0, &data);
    memcpy(data, vertices.data(), vertexSize);
    vkUnmapMemory(context_.device_, stagingBufferMemory);
    createVertexBuffer(vertexSize);
    copyBuffer(context_.device_,
        context_.graphicsQueue_,
        stagingCommandBuffer, 
        vertexBuffer_,
        stagingBuffer, 
        vertexSize);

    createIndexBuffer(indices.size()  *sizeof(uint16_t));
    memcpy(data, indices.data(), indexSize);
    copyBuffer(context_.device_,
        context_.graphicsQueue_,
        stagingCommandBuffer, 
        indexBuffer_,
        stagingBuffer, 
        indexSize);
    vkFreeCommandBuffers(context_.device_, context_.commandPool_, 1, &stagingCommandBuffer);
    
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
    frameStart();
    VkCommandBuffer commandBuffer = commandBuffers_[currentFrame];
    VkBuffer vertexBuffers[] = {vertexBuffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline_);
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