#include "renderer/MandelbrotRenderer.h"

#include <utility>
#include <shaderc/shaderc.hpp>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(shaders);

MandelbrotRenderer::MandelbrotRenderer(std::shared_ptr<vulkan::Context> context, int width, int height,
                                       int framesInFlight) : context(std::move(context)),
                                                             framesInFlight(framesInFlight),
                                                             imageWidth(width),
                                                             imageHeight(height){
    createComputeImage();
    createUniformBuffer();
    createDescriptorSetLayout();
    createComputePipeline();
    createCommandPool();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    imguiTexture = ImGui_ImplVulkan_AddTexture(accumulationImage.sampler,
                                               accumulationImage.imageView,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

MandelbrotRenderer::~MandelbrotRenderer() {
    for (int i = 0; i < commandPools.size(); ++i) {
        context->getDevice()->getVkDevice().freeCommandBuffers(commandPools[i], commandBuffers[i]);
        context->getDevice()->getVkDevice().destroyCommandPool(commandPools[i]);
    }
    for (auto descriptorPool: descriptorPools) {
        context->getDevice()->getVkDevice().destroyDescriptorPool(descriptorPool);
    }
    context->getDevice()->getVkDevice().destroyPipeline(pipeline);
    context->getDevice()->getVkDevice().destroyPipelineLayout(pipelineLayout);
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(descriptorSetLayout);

    context->getDevice()->getVkDevice().destroyImageView(accumulationImage.imageView);
    context->getDevice()->getVkDevice().destroySampler(accumulationImage.sampler);
    vmaDestroyImage(context->getAllocator(), accumulationImage.image, accumulationImage.allocation);
    vmaDestroyBuffer(context->getAllocator(), uniformBuffer.buffer, uniformBuffer.allocation);
}

RecordedCommandBuffer MandelbrotRenderer::recordCommandBuffer() {
    imageIndex = (imageIndex + 1) % framesInFlight;
    updateUniformBuffer();

    context->getDevice()->getVkDevice().resetCommandPool(commandPools[imageIndex]);

    auto commandBuffer = commandBuffers[imageIndex];
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::ImageMemoryBarrier imageMemoryBarrier{
            {},
            {},
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eGeneral,
            {},
            {},
            accumulationImage.image,
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

    imageMemoryBarrier = {
            {},
            {},
            vk::ImageLayout::eGeneral,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            {},
            {},
            accumulationImage.image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
    };

    commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            vk::DependencyFlagBits::eByRegion,
            nullptr,
            nullptr,
            imageMemoryBarrier
    );

    commandBuffer.end();

    return {context->getDevice()->getComputeQueue(), commandBuffer};
}

void MandelbrotRenderer::resizeImage(int width, int height) {
    imageWidth = width;
    imageHeight = height;

    context->getDevice()->getVkDevice().destroyImageView(accumulationImage.imageView);
    context->getDevice()->getVkDevice().destroySampler(accumulationImage.sampler);
    vmaDestroyImage(context->getAllocator(), accumulationImage.image, accumulationImage.allocation);

    createComputeImage();
    updateDescriptorSets();

    imguiTexture = ImGui_ImplVulkan_AddTexture(accumulationImage.sampler,
                                               accumulationImage.imageView,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void MandelbrotRenderer::updateDescriptorSets() {
    for (int i = 0; i < framesInFlight; i++) {
        vk::DescriptorImageInfo descriptorImageInfo(accumulationImage.sampler, accumulationImage.imageView,
                                                    vk::ImageLayout::eGeneral);

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets{};
        writeDescriptorSets.push_back(
                {descriptorSets[i],
                 0,
                 {},
                 vk::DescriptorType::eStorageImage,
                 descriptorImageInfo}
        );

        vk::DescriptorBufferInfo descriptorBufferInfo(uniformBuffer.buffer, {}, sizeof(MandelbrotUniformCamObj));
        writeDescriptorSets.push_back({descriptorSets[i],
                                       1,
                                       {},
                                       vk::DescriptorType::eUniformBuffer,
                                       {}, descriptorBufferInfo}
        );
        context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}

void MandelbrotRenderer::updateUniformBuffer() {
    void *data;
    vmaMapMemory(context->getAllocator(), uniformBuffer.allocation, &data);
    std::memcpy(data, &ubo, sizeof(MandelbrotUniformCamObj));
    vmaUnmapMemory(context->getAllocator(), uniformBuffer.allocation);
}


// ==================
// Creation functions
// ==================

void MandelbrotRenderer::createComputeImage() {
    VmaAllocation allocation;
    vk::Image image;
    vk::Sampler sampler;
    vk::ImageView imageView;

    vk::ImageCreateInfo imageCreateInfo{
            {},
            vk::ImageType::e2D,
            vk::Format::eB8G8R8A8Unorm,
            {static_cast<uint32_t>(imageWidth), static_cast<uint32_t>(imageHeight), 1},
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
                vk::ImageLayout::eShaderReadOnlyOptimal,
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
    samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
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

    accumulationImage = {allocation, image, sampler, imageView};
}

void MandelbrotRenderer::createUniformBuffer() {
    vk::BufferCreateInfo createInfo{
            {},
            sizeof(MandelbrotUniformCamObj),
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

void MandelbrotRenderer::createDescriptorSetLayout() {
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

void MandelbrotRenderer::createComputePipeline() {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto fs = cmrc::shaders::get_filesystem();
    auto shaderResource = fs.open("shaders/compute/mandelbrot.comp");
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

    auto dsl = {descriptorSetLayout};
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            {}, dsl
    };

    pipelineLayout = context->getDevice()->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

    vk::ComputePipelineCreateInfo computePipelineCreateInfo{
            {}, shaderStageCreateInfo, pipelineLayout
    };

    pipeline = context->getDevice()->getVkDevice().createComputePipeline(nullptr, computePipelineCreateInfo).value;

    context->getDevice()->getVkDevice().destroyShaderModule(shaderModule);
}

void MandelbrotRenderer::createCommandPool() {
    for (int i = 0; i < framesInFlight; ++i) {
        commandPools.emplace_back(
                context->getDevice()->getVkDevice().createCommandPool(
                        vk::CommandPoolCreateInfo({}, context->getDevice()->getComputeQueueFamily())));

    }
}

void MandelbrotRenderer::createDescriptorPool() {
    for (int i = 0; i < 3; i++) {
        std::vector<vk::DescriptorPoolSize> poolSizes{
                {vk::DescriptorType::eStorageImage,         1},
                {vk::DescriptorType::eCombinedImageSampler, 4},
                {vk::DescriptorType::eStorageBuffer,        10}
        };

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
                {},
                3,
                poolSizes
        );

        descriptorPools.push_back(context->getDevice()->getVkDevice().createDescriptorPool(descriptorPoolCreateInfo));
    }
}

void MandelbrotRenderer::createDescriptorSets() {
    for (int i = 0; i < framesInFlight; i++) {
        vk::DescriptorSetAllocateInfo allocateInfo{descriptorPools[i], descriptorSetLayout};
        descriptorSets.push_back(context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo).at(0));
    }
    updateDescriptorSets();
}

void MandelbrotRenderer::createCommandBuffers() {
    commandBuffers.resize(framesInFlight);
    for (int i = 0; i < framesInFlight; ++i) {
        commandBuffers[i] = context->getDevice()->getVkDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(commandPools[i], vk::CommandBufferLevel::ePrimary, 1))[0];
    }
}
