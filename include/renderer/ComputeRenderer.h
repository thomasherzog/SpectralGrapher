#ifndef SPECTRALGRAPHER_COMPUTERENDERER_H
#define SPECTRALGRAPHER_COMPUTERENDERER_H

#include "graphics/vulkan/core/Context.h"
#include "renderer/RecordedCommandBuffer.h"

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/euler_angles.hpp>

struct AllocatedImage {
    VmaAllocation allocation;
    vk::Image image;
    vk::Sampler sampler;
    vk::ImageView imageView;
};

struct AllocatedBuffer {
    VmaAllocation allocation;
    vk::Buffer buffer;
};

struct UniformCamObj {
    alignas(16) glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    alignas(16) glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    alignas(4) float fov = 0.0f;
};

class ComputeRenderer {

public:
    ComputeRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int imagesInFlight);

    ~ComputeRenderer();

    RecordedCommandBuffer recordCommandBuffer();

    void resizeImage(int width, int height);

    void updateDescriptorSets(int imagesInFlight);

    void updateUniformBuffer();

    int imageIndex;

    std::vector<AllocatedImage> images;

    vk::Semaphore semaphore;

    std::vector<vk::DescriptorPool> descriptorPools;

    AllocatedBuffer uniformBuffer;
    UniformCamObj ubo;

private:
    std::shared_ptr<vulkan::Context> context;

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::PipelineLayout pipelineLayout;

    vk::Pipeline pipeline;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::DescriptorSet> descriptorSets;

    void createComputeImages(int width, int height, int imagesInFlight);

    void createUniformBuffer();

    void createDescriptorSetLayout();

    void createComputePipeline();

    void createCommandPool(int imagesInFlight);

    void createDescriptorPool();

    void createDescriptorSets(int imagesInFlight);

    void createCommandBuffers(int imagesInFlight);

};


#endif //SPECTRALGRAPHER_COMPUTERENDERER_H
