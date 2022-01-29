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

struct SignedDistanceField {
    alignas(4) int type = -1;
    alignas(4) int id = -1;
    alignas(4) int dataOffset = -1;
};

struct Sphere {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float radius = 1.0f;
};

struct Mandelbulb {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float power = 8.0f;
};

struct UniformCamObj {
    alignas(16) glm::vec3 position = glm::vec3(-2.0f, 0.0f, 0.0f);
    alignas(16) glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    alignas(4) float fov = 1.0f;
    alignas(4) int sampleCount = 1;
    alignas(4) int sampleIndex = 0;
    alignas(4) float epsilon = 0.001;
    alignas(4) int maxTime = 7;
    alignas(4) int maxSteps = 200;
    alignas(16) glm::vec3 backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);
};

class ComputeRenderer {

public:
    ComputeRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int imagesInFlight);

    ~ComputeRenderer();

    RecordedCommandBuffer recordCommandBuffer();

    void resizeImage(int width, int height);

    void updateDescriptorSets();

    void updateUniformBuffer();

    int imageIndex{-1};
    int framesInFlight;

    AllocatedImage accumulationImage;

    vk::Semaphore semaphore;

    std::vector<vk::DescriptorPool> descriptorPools;

    AllocatedBuffer uniformBuffer;
    UniformCamObj ubo;

    std::vector<SignedDistanceField> sdfs;
    std::vector<Sphere> spheres;
    std::vector<Mandelbulb> mandelbulbs;

    AllocatedBuffer sdfBuffer;
    AllocatedBuffer sphereBuffer;
    AllocatedBuffer mandelbulbBuffer;

    vk::DescriptorSetLayout objectDSL;
    std::vector<vk::DescriptorSet> descriptorSetsObj;
private:
    std::shared_ptr<vulkan::Context> context;

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::PipelineLayout pipelineLayout;

    vk::Pipeline pipeline;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::DescriptorSet> descriptorSets;

    void createComputeImage(int width, int height);

    void createUniformBuffer();

    void createDescriptorSetLayout();

    void createComputePipeline();

    void createCommandPool();

    void createDescriptorPool();

    void createDescriptorSets();

    void createCommandBuffers();

};


#endif //SPECTRALGRAPHER_COMPUTERENDERER_H
