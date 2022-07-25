#ifndef SPECTRALGRAPHER_MANDELBROTRENDERER_H
#define SPECTRALGRAPHER_MANDELBROTRENDERER_H

#include "graphics/vulkan/core/Context.h"
#include "renderer/general/RecordedCommandBuffer.h"
#include "renderer/ComputeRenderer.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/euler_angles.hpp>

struct MandelbrotUniformCamObj {
    alignas(16) glm::vec4 bounds = glm::vec4(-2, 1, -1.5, 1.5);
    alignas(8) glm::vec2 center = glm::vec2(0, 0);
    alignas(8) glm::vec2 viewportSize = glm::vec2(0,0);
    alignas(4) float scale = 1.0;
    alignas(4) int iterations = 100;
    alignas(8) glm::vec2 cValue = glm::vec2(-0.6,0.6);
};

class MandelbrotRenderer {

public:
    MandelbrotRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int framesInFlight);

    ~MandelbrotRenderer();

    RecordedCommandBuffer recordCommandBuffer();

    void resizeImage(int width, int height);

    void updateDescriptorSets();

    void updateUniformBuffer();

    MandelbrotUniformCamObj ubo;

    ImTextureID imguiTexture;
    int imageWidth;
    int imageHeight;

private:
    std::shared_ptr<vulkan::Context> context;

    int imageIndex{-1};
    int framesInFlight;

    AllocatedImage accumulationImage;

    std::vector<vk::DescriptorPool> descriptorPools;

    AllocatedBuffer uniformBuffer;

    vk::DescriptorSetLayout descriptorSetLayout;

    vk::PipelineLayout pipelineLayout;

    vk::Pipeline pipeline;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::DescriptorSet> descriptorSets;

    void createComputeImage();

    void createUniformBuffer();

    void createDescriptorSetLayout();

    void createComputePipeline();

    void createCommandPool();

    void createDescriptorPool();

    void createDescriptorSets();

    void createCommandBuffers();

};


#endif //SPECTRALGRAPHER_MANDELBROTRENDERER_H
