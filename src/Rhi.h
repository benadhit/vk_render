#pragma once

typedef void *RhiBuffer;
typedef void *RhiTexture;
typedef void *RhiShader;
typedef void *RhiPipeline;
typedef void *RhiCommandBuffer;
typedef void *RhiCommadPool;

enum BufferType
{
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_UNIFORM,

    BUFFER_TYPE_MAX,
};

enum BufferStoragePolicy
{
    BUFFER_STORAG_POLITY_SHARED_CPU_AND_GPU,
    BUFFER_STORAG_POLITY_GPU_ONLY,
    BUFFER_STORAG_POLITY_GPU_MAX,
};

extern struct __RHI_API__
{
    RhiBuffer (*createBuffer)(BufferType bufferType,
                              BufferStoragePolicy storagePolicy, size_t size);
    RhiTexture (*createTexture)();
    RhiShader (*createShader)();
    RhiPipeline (*createPipeline)();
    RhiCommandBuffer (*createCommandBuffer)();
    RhiCommadPool (*createCommandPool)();
} Rhi;