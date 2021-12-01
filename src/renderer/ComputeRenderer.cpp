#include "renderer/ComputeRenderer.h"

#include <fstream>
#include <utility>
#include <shaderc/shaderc.hpp>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(shaders);

ComputeRenderer::ComputeRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int imagesInFlight)
        : context(std::move(context)) {
    createComputeImages(width, height, imagesInFlight);
    createUniformBuffer();
    createDescriptorSetLayout();
    createComputePipeline();
    createCommandPool(imagesInFlight);
    createDescriptorPool();
    createDescriptorSets(imagesInFlight);
    createCommandBuffers(imagesInFlight);
}

ComputeRenderer::~ComputeRenderer() {
    for (int i = 0; i < commandPools.size(); ++i) {
        context->getDevice()->getVkDevice().freeCommandBuffers(commandPools[i], commandBuffers[i]);
        context->getDevice()->getVkDevice().destroyCommandPool(commandPools[i]);
    }
    context->getDevice()->getVkDevice().destroySemaphore(semaphore);
    for (auto descriptorPool: descriptorPools) {
        context->getDevice()->getVkDevice().destroyDescriptorPool(descriptorPool);
    }
    context->getDevice()->getVkDevice().destroyPipeline(pipeline);
    context->getDevice()->getVkDevice().destroyPipelineLayout(pipelineLayout);
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    for (auto allocatedImage: images) {
        context->getDevice()->getVkDevice().destroyImageView(allocatedImage.imageView);
        context->getDevice()->getVkDevice().destroySampler(allocatedImage.sampler);
        vmaDestroyImage(context->getAllocator(), allocatedImage.image, allocatedImage.allocation);
    }
    vmaDestroyBuffer(context->getAllocator(), uniformBuffer.buffer, uniformBuffer.allocation);
}

RecordedCommandBuffer ComputeRenderer::recordCommandBuffer() {
    imageIndex = (imageIndex + 1) % images.size();

    context->getDevice()->getVkDevice().resetCommandPool(commandPools[imageIndex]);
    auto commandBuffer = commandBuffers[imageIndex];

    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::ImageMemoryBarrier imageMemoryBarrier{
            {},
            {},
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eGeneral,
            {},
            {},
            images[imageIndex].image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
    };

    commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion,
            nullptr,
            nullptr,
            imageMemoryBarrier
    );
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0,
                                     descriptorSets[imageIndex], nullptr);
    commandBuffer.dispatch(200, 200, 1);

    commandBuffer.end();

    return {context, commandBuffer};
}


void ComputeRenderer::resizeImage(int width, int height) {
    int imagesInFlight = images.size();
    for (auto allocatedImage: images) {
        context->getDevice()->getVkDevice().destroyImageView(allocatedImage.imageView);
        context->getDevice()->getVkDevice().destroySampler(allocatedImage.sampler);
        vmaDestroyImage(context->getAllocator(), allocatedImage.image, allocatedImage.allocation);
    }
    images.clear();
    createComputeImages(width, height, imagesInFlight);
    updateUniformBuffer();
    updateDescriptorSets(imagesInFlight);
}



// ==================
// Creation functions
// ==================

void ComputeRenderer::createComputeImages(int width, int height, int imagesInFlight) {
    for (int i = 0; i < imagesInFlight; i++) {
        VmaAllocation allocation;
        vk::Image image;
        vk::Sampler sampler;
        vk::ImageView imageView;

        vk::ImageCreateInfo imageCreateInfo{
                {},
                vk::ImageType::e2D,
                vk::Format::eB8G8R8A8Unorm,
                {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
                1,
                1,
                vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
                {},
                nullptr,
                vk::ImageLayout::eUndefined
        };

        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        vmaCreateImage(context->getAllocator(),
                       reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                       &allocationInfo,
                       reinterpret_cast<VkImage *>(&image),
                       &allocation,
                       nullptr);

        context->executeTransient([&image = image](vk::CommandBuffer commandBuffer) {
            vk::ImageMemoryBarrier imageMemoryBarrier{
                    {},
                    {},
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eGeneral,
                    {},
                    {},
                    image,
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
            };

            commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllCommands,
                    vk::PipelineStageFlagBits::eAllCommands,
                    {},
                    nullptr,
                    nullptr,
                    imageMemoryBarrier
            );
            return 1;
        });

        vk::SamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.magFilter = vk::Filter::eLinear;
        samplerCreateInfo.minFilter = vk::Filter::eLinear;
        samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToBorder;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToBorder;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToBorder;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.compareOp = vk::CompareOp::eNever;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;
        samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        sampler = context->getDevice()->getVkDevice().createSampler(samplerCreateInfo);

        vk::ImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.format = vk::Format::eB8G8R8A8Unorm;
        imageViewCreateInfo.components = {
                vk::ComponentSwizzle::eR,
                vk::ComponentSwizzle::eG,
                vk::ComponentSwizzle::eB,
                vk::ComponentSwizzle::eA
        };
        imageViewCreateInfo.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
        imageViewCreateInfo.image = image;
        imageView = context->getDevice()->getVkDevice().createImageView(imageViewCreateInfo);

        images.push_back({allocation, image, sampler, imageView});
    }
}


void ComputeRenderer::createUniformBuffer() {
    vk::BufferCreateInfo createInfo{
            {},
            sizeof(UniformCamObj),
            vk::BufferUsageFlagBits::eUniformBuffer,
            {}, nullptr
    };

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
            context->getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&createInfo),
            &allocationInfo,
            reinterpret_cast<VkBuffer *>(&uniformBuffer.buffer),
            &uniformBuffer.allocation,
            nullptr
    );
    updateUniformBuffer();
}


void ComputeRenderer::createDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{
            vk::DescriptorSetLayoutBinding{
                    0,
                    vk::DescriptorType::eStorageImage,
                    1,
                    vk::ShaderStageFlagBits::eCompute
            },
            vk::DescriptorSetLayoutBinding{
                    1,
                    vk::DescriptorType::eUniformBuffer,
                    1,
                    vk::ShaderStageFlagBits::eCompute
            }
    };

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, setLayoutBindings};
    descriptorSetLayout = context->getDevice()->getVkDevice().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
}

void ComputeRenderer::createComputePipeline() {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto fs = cmrc::shaders::get_filesystem();
    auto shaderResource = fs.open("shaders/compute/test.comp");
    std::string shaderSource{shaderResource.begin(), shaderResource.end()};
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            shaderSource,
            shaderc_glsl_compute_shader,
            "name",
            options
    );
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
    }
    std::vector<uint32_t> spirv{result.begin(), result.end()};
    vk::ShaderModule shaderModule = context->getDevice()->getVkDevice().createShaderModule(vk::ShaderModuleCreateInfo{
            {}, spirv.size() * sizeof(uint32_t), spirv.data()
    });

    vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{
            {}, vk::ShaderStageFlagBits::eCompute, shaderModule, "main"
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            {}, descriptorSetLayout
    };

    pipelineLayout = context->getDevice()->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

    vk::ComputePipelineCreateInfo computePipelineCreateInfo{
            {}, shaderStageCreateInfo, pipelineLayout
    };

    pipeline = context->getDevice()->getVkDevice().createComputePipeline(nullptr, computePipelineCreateInfo).value;

    context->getDevice()->getVkDevice().destroyShaderModule(shaderModule);
}

void ComputeRenderer::createCommandPool(int imagesInFlight) {
    for (int i = 0; i < imagesInFlight; ++i) {
        commandPools.emplace_back(
                context->getDevice()->getVkDevice().createCommandPool(
                        vk::CommandPoolCreateInfo({}, context->getDevice()->getGraphicsQueueFamily())));

    }
}

void ComputeRenderer::createDescriptorPool() {
    for (int i = 0; i < 3; i++) {
        std::vector<vk::DescriptorPoolSize> poolSizes{
                {vk::DescriptorType::eStorageImage,         1},
                {vk::DescriptorType::eCombinedImageSampler, 4}
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
                {},
                3,
                poolSizes
        );

        descriptorPools.push_back(context->getDevice()->getVkDevice().createDescriptorPool(descriptorPoolCreateInfo));
    }
}

void ComputeRenderer::createDescriptorSets(int imagesInFlight) {
    for (int i = 0; i < imagesInFlight; i++) {
        vk::DescriptorSetAllocateInfo allocateInfo{descriptorPools[i], descriptorSetLayout};
        descriptorSets.push_back(context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo).at(0));
    }
    updateDescriptorSets(imagesInFlight);
}

void ComputeRenderer::updateDescriptorSets(int imagesInFlight) {
    for (int i = 0; i < imagesInFlight; i++) {
        auto image = images[i];
        vk::DescriptorImageInfo descriptorImageInfo(image.sampler, image.imageView, vk::ImageLayout::eGeneral);

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
        writeDescriptorSets.push_back(
                {descriptorSets[i],
                 0,
                 {},
                 vk::DescriptorType::eStorageImage,
                 descriptorImageInfo}
        );

        vk::DescriptorBufferInfo descriptorBufferInfo(uniformBuffer.buffer, {}, sizeof(UniformCamObj));
        writeDescriptorSets.push_back({descriptorSets[i],
                                       1,
                                       {},
                                       vk::DescriptorType::eUniformBuffer,
                                       {}, descriptorBufferInfo}
        );

        context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}

void ComputeRenderer::createCommandBuffers(int imagesInFlight) {
    commandBuffers.resize(imagesInFlight);
    for (int i = 0; i < imagesInFlight; ++i) {
        commandBuffers[i] = context->getDevice()->getVkDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(commandPools[i], vk::CommandBufferLevel::ePrimary, 1))[0];
    }

    semaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
}

void ComputeRenderer::updateUniformBuffer() {
    void *data;
    vmaMapMemory(context->getAllocator(), uniformBuffer.allocation, &data);
    std::memcpy(data, &ubo, sizeof(UniformCamObj));
    vmaUnmapMemory(context->getAllocator(), uniformBuffer.allocation);
}



