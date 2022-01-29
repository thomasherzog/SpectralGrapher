#include "renderer/ComputeRenderer.h"

#include <utility>
#include <shaderc/shaderc.hpp>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(shaders);

ComputeRenderer::ComputeRenderer(std::shared_ptr<vulkan::Context> context, int width, int height, int framesInFlight)
        : context(std::move(context)), framesInFlight(framesInFlight) {

    spheres.push_back({glm::vec3{0, 1, 0}, 0.25f});
    spheres.push_back({glm::vec3{0, 2, 0}, 0.35f});
    spheres.push_back({glm::vec3{0, 3, 0}, 0.45f});

    mandelbulbs.push_back({glm::vec3{1, 1, 1}, 8.0f});

    sdfs.clear();
    {
        for(int i = 0; i < spheres.size(); i++) {
            sdfs.push_back({1, i,1});
        }
        for(int i = 0; i < mandelbulbs.size(); i++) {
            sdfs.push_back({2, i,1});
        }
    }

    createComputeImage(width, height);
    createUniformBuffer();
    createDescriptorSetLayout();
    createComputePipeline();
    createCommandPool();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
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
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(objectDSL);

    context->getDevice()->getVkDevice().destroyImageView(accumulationImage.imageView);
    context->getDevice()->getVkDevice().destroySampler(accumulationImage.sampler);
    vmaDestroyImage(context->getAllocator(), accumulationImage.image, accumulationImage.allocation);

    vmaDestroyBuffer(context->getAllocator(), uniformBuffer.buffer, uniformBuffer.allocation);
    vmaDestroyBuffer(context->getAllocator(), sdfBuffer.buffer, sdfBuffer.allocation);
    vmaDestroyBuffer(context->getAllocator(), sphereBuffer.buffer, sphereBuffer.allocation);
    vmaDestroyBuffer(context->getAllocator(), mandelbulbBuffer.buffer, mandelbulbBuffer.allocation);
}

RecordedCommandBuffer ComputeRenderer::recordCommandBuffer() {
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

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), sdfBuffer.allocation, &objectData);

        auto *objectSSBO = (SignedDistanceField *) objectData;

        for (int i = 0; i < sdfs.size(); i++) {
            SignedDistanceField &object = sdfs[i];
            objectSSBO[i].type = object.type;
            objectSSBO[i].id = object.id;
        }

        vmaUnmapMemory(context->getAllocator(), sdfBuffer.allocation);
    }

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), sphereBuffer.allocation, &objectData);

        Sphere *objectSSBO = (Sphere *) objectData;

        for (int i = 0; i < spheres.size(); i++) {
            Sphere &object = spheres[i];
            objectSSBO[i].position = object.position;
            objectSSBO[i].radius = object.radius;
        }

        vmaUnmapMemory(context->getAllocator(), sphereBuffer.allocation);
    }

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), mandelbulbBuffer.allocation, &objectData);

        Mandelbulb *objectSSBO = (Mandelbulb *) objectData;

        for (int i = 0; i < mandelbulbs.size(); i++) {
            Mandelbulb &object = mandelbulbs[i];
            objectSSBO[i].position = object.position;
            objectSSBO[i].power = object.power;
        }

        vmaUnmapMemory(context->getAllocator(), mandelbulbBuffer.allocation);
    }

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 1,
                                     descriptorSetsObj[imageIndex], nullptr);

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

    ubo.sampleIndex += ubo.sampleCount;

    return {context->getDevice()->getComputeQueue(), commandBuffer};
}


void ComputeRenderer::resizeImage(int width, int height) {
    ubo.sampleIndex = 0;

    context->getDevice()->getVkDevice().destroyImageView(accumulationImage.imageView);
    context->getDevice()->getVkDevice().destroySampler(accumulationImage.sampler);
    vmaDestroyImage(context->getAllocator(), accumulationImage.image, accumulationImage.allocation);

    createComputeImage(width, height);
    updateDescriptorSets();
}



// ==================
// Creation functions
// ==================

void ComputeRenderer::createComputeImage(int width, int height) {
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

    // SDF BUFFER
    createInfo = {
            {},
            sdfs.size() * sizeof(SignedDistanceField),
            vk::BufferUsageFlagBits::eStorageBuffer,
            {}, nullptr
    };

    allocationInfo = {};
    allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
            context->getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&createInfo),
            &allocationInfo,
            reinterpret_cast<VkBuffer *>(&sdfBuffer.buffer),
            &sdfBuffer.allocation,
            nullptr
    );
    // END SDF BUFFER

    createInfo = {
            {},
            spheres.size() * sizeof(Sphere),
            vk::BufferUsageFlagBits::eStorageBuffer,
            {}, nullptr
    };

    allocationInfo = {};
    allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
            context->getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&createInfo),
            &allocationInfo,
            reinterpret_cast<VkBuffer *>(&sphereBuffer.buffer),
            &sphereBuffer.allocation,
            nullptr
    );

    createInfo = {
            {},
            mandelbulbs.size() * sizeof(Mandelbulb),
            vk::BufferUsageFlagBits::eStorageBuffer,
            {}, nullptr
    };
    vmaCreateBuffer(
            context->getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&createInfo),
            &allocationInfo,
            reinterpret_cast<VkBuffer *>(&mandelbulbBuffer.buffer),
            &mandelbulbBuffer.allocation,
            nullptr
    );

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), sdfBuffer.allocation, &objectData);

        auto *objectSSBO = (SignedDistanceField *) objectData;

        for (int i = 0; i < sdfs.size(); i++) {
            SignedDistanceField &object = sdfs[i];
            objectSSBO[i].type = object.type;
            objectSSBO[i].id = object.id;
        }

        vmaUnmapMemory(context->getAllocator(), sdfBuffer.allocation);
    }

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), sphereBuffer.allocation, &objectData);

        auto *objectSSBO = (Sphere *) objectData;

        for (int i = 0; i < spheres.size(); i++) {
            Sphere &object = spheres[i];
            objectSSBO[i].position = object.position;
            objectSSBO[i].radius = object.radius;
        }

        vmaUnmapMemory(context->getAllocator(), sphereBuffer.allocation);
    }

    {
        void *objectData;
        vmaMapMemory(context->getAllocator(), mandelbulbBuffer.allocation, &objectData);

        auto *objectSSBO = (Mandelbulb *) objectData;

        for (int i = 0; i < mandelbulbs.size(); i++) {
            Mandelbulb &object = mandelbulbs[i];
            objectSSBO[i].position = object.position;
            objectSSBO[i].power = object.power;
        }

        vmaUnmapMemory(context->getAllocator(), mandelbulbBuffer.allocation);
    }

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


    std::vector<vk::DescriptorSetLayoutBinding> setObjLayoutBindings{
            vk::DescriptorSetLayoutBinding{
                    0,
                    vk::DescriptorType::eStorageBuffer,
                    1,
                    vk::ShaderStageFlagBits::eCompute
            },
            vk::DescriptorSetLayoutBinding{
                    1,
                    vk::DescriptorType::eStorageBuffer,
                    1,
                    vk::ShaderStageFlagBits::eCompute
            },
            vk::DescriptorSetLayoutBinding{
                    2,
                    vk::DescriptorType::eStorageBuffer,
                    1,
                    vk::ShaderStageFlagBits::eCompute
            }
    };

    vk::DescriptorSetLayoutCreateInfo descriptorSetObjLayoutCreateInfo{{}, setObjLayoutBindings};
    objectDSL = context->getDevice()->getVkDevice().createDescriptorSetLayout(descriptorSetObjLayoutCreateInfo);
}

void ComputeRenderer::createComputePipeline() {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto fs = cmrc::shaders::get_filesystem();
    auto shaderResource = fs.open("shaders/compute/raymarcher.comp");
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

    auto dsl = {descriptorSetLayout, objectDSL};
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

void ComputeRenderer::createCommandPool() {
    for (int i = 0; i < framesInFlight; ++i) {
        commandPools.emplace_back(
                context->getDevice()->getVkDevice().createCommandPool(
                        vk::CommandPoolCreateInfo({}, context->getDevice()->getComputeQueueFamily())));

    }
}

void ComputeRenderer::createDescriptorPool() {
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

void ComputeRenderer::createDescriptorSets() {
    for (int i = 0; i < framesInFlight; i++) {
        vk::DescriptorSetAllocateInfo allocateInfo{descriptorPools[i], descriptorSetLayout};
        descriptorSets.push_back(context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo).at(0));

        vk::DescriptorSetAllocateInfo allocateInfoObj{descriptorPools[i], objectDSL};
        descriptorSetsObj.push_back(context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfoObj).at(0));
    }
    updateDescriptorSets();
}

void ComputeRenderer::updateDescriptorSets() {
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

        vk::DescriptorBufferInfo descriptorBufferInfo(uniformBuffer.buffer, {}, sizeof(UniformCamObj));
        writeDescriptorSets.push_back({descriptorSets[i],
                                       1,
                                       {},
                                       vk::DescriptorType::eUniformBuffer,
                                       {}, descriptorBufferInfo}
        );

        vk::DescriptorBufferInfo descriptorSDFInfo(sdfBuffer.buffer, {}, sdfs.size() * sizeof(SignedDistanceField));
        writeDescriptorSets.push_back({descriptorSetsObj[i],
                                       0,
                                       {},
                                       vk::DescriptorType::eStorageBuffer,
                                       {}, descriptorSDFInfo}
        );

        vk::DescriptorBufferInfo descriptorSphereInfo(sphereBuffer.buffer, {}, spheres.size() * sizeof(Sphere));
        writeDescriptorSets.push_back({descriptorSetsObj[i],
                                       1,
                                       {},
                                       vk::DescriptorType::eStorageBuffer,
                                       {}, descriptorSphereInfo}
        );

        vk::DescriptorBufferInfo descriptorMandelbulbInfo(mandelbulbBuffer.buffer, {},
                                                     mandelbulbs.size() * sizeof(Mandelbulb));
        writeDescriptorSets.push_back({descriptorSetsObj[i],
                                       2,
                                       {},
                                       vk::DescriptorType::eStorageBuffer,
                                       {}, descriptorMandelbulbInfo}
        );

        context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}

void ComputeRenderer::createCommandBuffers() {
    commandBuffers.resize(framesInFlight);
    for (int i = 0; i < framesInFlight; ++i) {
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

