#ifndef SPECTRALGRAPHER_COMPUTERENDERER_H
#define SPECTRALGRAPHER_COMPUTERENDERER_H

#include "graphics/vulkan/core/Context.h"
#include "renderer/RecordedCommandBuffer.h"

struct AllocatedImage {
    VmaAllocation allocation;
    vk::Image image;
    vk::Sampler sampler;
    vk::ImageView imageView;
};

class ComputeRenderer {

public:
    ComputeRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int imagesInFlight);

    ~ComputeRenderer();

    RecordedCommandBuffer recordCommandBuffer();

    void resizeImage(int width, int height);

    void updateDescriptorSets(int imagesInFlight);

    int imageIndex;

    //TODO: texture class
    std::vector<AllocatedImage> images;

    vk::Semaphore semaphore;

    std::vector<vk::DescriptorPool> descriptorPools;


private:
    std::shared_ptr<vulkan::Context> context;

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::PipelineLayout pipelineLayout;

    vk::Pipeline pipeline;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::DescriptorSet> descriptorSets;

    void createComputeImages(int width, int height, int imagesInFlight);

    void createDescriptorSetLayout();

    void createComputePipeline();

    void createCommandPool(int imagesInFlight);

    void createDescriptorPool();

    void createDescriptorSets(int imagesInFlight);

    void createCommandBuffers(int imagesInFlight);

};


#endif //SPECTRALGRAPHER_COMPUTERENDERER_H
