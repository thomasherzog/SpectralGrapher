#ifndef SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
#define SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H

#include "windowing/VulkanWindow.h"

class SimpleRasterizerWindowEx : public windowing::VulkanWindow {
public:
    SimpleRasterizerWindowEx();

    ~SimpleRasterizerWindowEx();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onSwapchainRebuild() override;

private:

    struct ComputeRenderer {
        vk::DescriptorSetLayout descriptorSetLayout;

        vk::PipelineLayout pipelineLayout;

        vk::Pipeline pipeline;

        std::vector<vk::CommandPool> commandPools;

        std::vector<vk::CommandBuffer> commandBuffers;

        vk::DescriptorPool descriptorPool;

        vk::DescriptorSet descriptorSet;

        vk::Image image;
        VmaAllocation imageAllocation;

        vk::Sampler imageSampler;
        vk::ImageView imageView;

        vk::Semaphore semaphore;
    } compute;

    void createComputeImage();

    void createDescriptorSetLayout();

    void createComputePipeline();

    void createComputeCommandPool();

    void createDescriptorPool();

    void createDescriptorSets();

    void createCommandBuffers();


    struct BasicGraphicsRenderer {
        vk::RenderPass renderPass;

        vk::DescriptorSetLayout descriptorSetLayout;

        vk::PipelineLayout pipelineLayout;

        vk::Pipeline pipeline;

        vk::DescriptorSet descriptorSet;

        std::vector<vk::Framebuffer> framebuffers;
    } graphics;

    void createRenderPass();

    void createFramebuffers();

    void createGraphicsDescriptorSetLayout();

    void createGraphicsDescriptorSet();

    void createRasterizer();

};


#endif //SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
