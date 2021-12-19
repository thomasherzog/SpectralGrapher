#ifndef SPECTRALGRAPHER_SWAPCHAINIMAGERENDERER_H
#define SPECTRALGRAPHER_SWAPCHAINIMAGERENDERER_H

#include "graphics/vulkan/core/Context.h"
#include "graphics/vulkan/core/Swapchain.h"
#include "renderer/RecordedCommandBuffer.h"
#include "renderer/ComputeRenderer.h"

class SwapchainImageRenderer {
public:
    SwapchainImageRenderer(std::shared_ptr<vulkan::Context> context, vulkan::Swapchain &swapchain);

    ~SwapchainImageRenderer();

    RecordedCommandBuffer recordCommandBuffer(vulkan::Swapchain &swapchain, int imageIndex, AllocatedImage image);

    void onSwapchainResize(vulkan::Swapchain &swapchain);

    void updateDescriptorSet(AllocatedImage image, int imageIndex);

private:
    std::shared_ptr<vulkan::Context> context;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    vk::RenderPass renderPass;

    vk::DescriptorSetLayout descriptorSetLayout;

    std::vector<vk::DescriptorPool> descriptorPools;

    vk::PipelineLayout pipelineLayout;

    vk::Pipeline pipeline;

    std::vector<vk::DescriptorSet> descriptorSets;

    std::vector<vk::Framebuffer> framebuffers;

    void createCommandPool(int imageCount);

    void createCommandBuffers(int imageCount);

    void createRenderPass(vulkan::Swapchain &swapchain);

    void createFramebuffers(vulkan::Swapchain &swapchain);

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSets();

    void createRasterizer();

};


#endif //SPECTRALGRAPHER_SWAPCHAINIMAGERENDERER_H
