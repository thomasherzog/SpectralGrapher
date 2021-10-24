#include "examples/SimpleRasterizerWindowEx.h"
#include <fstream>

SimpleRasterizerWindowEx::SimpleRasterizerWindowEx() {
    createComputeImage();
    createDescriptorSetLayout();
    createComputePipeline();
    createComputeCommandPool();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();

    createRenderPass();
    createFramebuffers();
    createGraphicsDescriptorSetLayout();
    createGraphicsDescriptorSet();
    createRasterizer();
}

SimpleRasterizerWindowEx::~SimpleRasterizerWindowEx() {
    context->getDevice()->getVkDevice().waitIdle();
    context->getDevice()->getVkDevice().destroyPipeline(graphics.pipeline);
    context->getDevice()->getVkDevice().destroyPipelineLayout(graphics.pipelineLayout);
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(graphics.descriptorSetLayout);
    for (auto &framebuffer: graphics.framebuffers) {
        context->getDevice()->getVkDevice().destroyFramebuffer(framebuffer);
    }
    graphics.framebuffers.clear();
    context->getDevice()->getVkDevice().destroyRenderPass(graphics.renderPass);
    for (int i = 0; i < compute.commandPools.size(); ++i) {
        context->getDevice()->getVkDevice().freeCommandBuffers(compute.commandPools[i], compute.commandBuffers[i]);
        context->getDevice()->getVkDevice().destroyCommandPool(compute.commandPools[i]);
    }
    context->getDevice()->getVkDevice().destroySemaphore(compute.semaphore);
    context->getDevice()->getVkDevice().destroyDescriptorPool(compute.descriptorPool);
    context->getDevice()->getVkDevice().destroyPipeline(compute.pipeline);
    context->getDevice()->getVkDevice().destroyPipelineLayout(compute.pipelineLayout);
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(compute.descriptorSetLayout);

    context->getDevice()->getVkDevice().destroyImageView(compute.imageView);
    context->getDevice()->getVkDevice().destroySampler(compute.imageSampler);
    vmaDestroyImage(context->getAllocator(), compute.image, compute.imageAllocation);
}

void SimpleRasterizerWindowEx::onSwapchainRebuild() {
    for (auto &framebuffer: graphics.framebuffers) {
        context->getDevice()->getVkDevice().destroyFramebuffer(framebuffer);
    }
    graphics.framebuffers.clear();
    context->getDevice()->getVkDevice().destroyRenderPass(graphics.renderPass);
    createRenderPass();
    createFramebuffers();
}

void SimpleRasterizerWindowEx::onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) {
    // Graphics command buffers
    context->getDevice()->getVkDevice().resetCommandPool(commandPools[imageIndex]);
    auto commandBuffer = commandBuffers[imageIndex];

    // Compute command buffers
    context->getDevice()->getVkDevice().resetCommandPool(compute.commandPools[imageIndex]);
    auto computeCommandBuffer = compute.commandBuffers[imageIndex];

    {
        computeCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        vk::ImageMemoryBarrier imageMemoryBarrier{
                {},
                {},
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eGeneral,
                {},
                {},
                compute.image,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        };

        computeCommandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlagBits::eByRegion,
                nullptr,
                nullptr,
                imageMemoryBarrier
        );
        computeCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, compute.pipeline);
        computeCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, compute.pipelineLayout, 0,
                                                compute.descriptorSet,
                                                nullptr);
        computeCommandBuffer.dispatch(200, 200, 1);

        computeCommandBuffer.end();


    }
    vk::SubmitInfo submitInfo(
            nullptr, nullptr,
            computeCommandBuffer,
            compute.semaphore
    );
    context->getDevice()->getComputeQueue().submit(submitInfo, nullptr);


    context->getDevice()->getVkDevice().resetFences(1, &syncObject.fence);

    // Graphics pipeline begin
    {
        commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        vk::ImageMemoryBarrier imageMemoryBarrier{
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eGeneral,
                vk::ImageLayout::eGeneral,
                {},
                {},
                compute.image,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        };
        commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eFragmentShader,
                {},
                nullptr,
                nullptr,
                imageMemoryBarrier
        );

        vk::ClearValue clearValue{};
        clearValue.color = std::array<float, 4>{1.0f, 0.0f, 1.0f, 1.0f};

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.renderPass = graphics.renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = swapchain->extent.width;
        renderPassBeginInfo.renderArea.extent.height = swapchain->extent.height;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;
        renderPassBeginInfo.framebuffer = graphics.framebuffers[imageIndex];

        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);


        vk::Viewport viewport{
                0, 0,
                static_cast<float>(swapchain->extent.width), static_cast<float>(swapchain->extent.height),
                0.0f, 1.0f
        };
        commandBuffer.setViewport(0, viewport);

        vk::Rect2D scissor{
                {0,                       0},
                {swapchain->extent.width, swapchain->extent.height}
        };
        commandBuffer.setScissor(0, scissor);


        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphics.pipelineLayout, 0,
                                         graphics.descriptorSet, nullptr);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics.pipeline);
        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();
    }
    // Graphics pipeline end


    std::vector<vk::PipelineStageFlags> pipelineStageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eFragmentShader};
    std::vector<vk::Semaphore> semaphores = {syncObject.imageAvailableSemaphore, compute.semaphore};
    context->getDevice()->getGraphicsQueue().submit(vk::SubmitInfo(
            semaphores,
            pipelineStageFlags,
            commandBuffer,
            syncObject.renderFinishedSemaphore
    ), syncObject.fence);
}

void SimpleRasterizerWindowEx::createDescriptorSetLayout() {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{
            vk::DescriptorSetLayoutBinding{0,
                                           vk::DescriptorType::eStorageImage,
                                           1,
                                           vk::ShaderStageFlagBits::eCompute}
    };

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, setLayoutBindings};
    compute.descriptorSetLayout = context->getDevice()->getVkDevice().createDescriptorSetLayout(
            descriptorSetLayoutCreateInfo);
}

void SimpleRasterizerWindowEx::createComputePipeline() {

    std::ifstream file("test.comp.spv", std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    vk::ShaderModule shaderModule = context->getDevice()->getVkDevice().createShaderModule(vk::ShaderModuleCreateInfo{
            {}, buffer.size(), reinterpret_cast<const uint32_t *>(buffer.data())
    });

    vk::PipelineShaderStageCreateInfo shaderStageCreateInfo{
            {}, vk::ShaderStageFlagBits::eCompute, shaderModule, "main"
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            {}, compute.descriptorSetLayout
    };

    compute.pipelineLayout = context->getDevice()->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);

    vk::ComputePipelineCreateInfo computePipelineCreateInfo{
            {}, shaderStageCreateInfo, compute.pipelineLayout
    };

    compute.pipeline = context->getDevice()->getVkDevice().createComputePipeline(nullptr,
                                                                                 computePipelineCreateInfo).value;

    context->getDevice()->getVkDevice().destroyShaderModule(shaderModule);
}

void SimpleRasterizerWindowEx::createComputeCommandPool() {
    for (int i = 0; i < swapchain->imageCount; ++i) {
        compute.commandPools.emplace_back(
                context->getDevice()->getVkDevice().createCommandPool(
                        vk::CommandPoolCreateInfo({}, context->getDevice()->getComputeQueueFamily())));
    }
}

void SimpleRasterizerWindowEx::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes{
            {vk::DescriptorType::eStorageImage,         1},
            {vk::DescriptorType::eCombinedImageSampler, 4}
    };

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
            {},
            3,
            poolSizes
    );

    compute.descriptorPool = context->getDevice()->getVkDevice().createDescriptorPool(descriptorPoolCreateInfo);
}

void SimpleRasterizerWindowEx::createDescriptorSets() {

    vk::DescriptorSetAllocateInfo allocateInfo(
            compute.descriptorPool,
            compute.descriptorSetLayout
    );

    compute.descriptorSet = context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo)[0];

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

    vk::DescriptorImageInfo descriptorImageInfo(compute.imageSampler, compute.imageView, vk::ImageLayout::eGeneral);
    vk::WriteDescriptorSet writeDescriptorSet(compute.descriptorSet,
                                              0,
                                              0,
                                              vk::DescriptorType::eStorageImage,
                                              descriptorImageInfo);
    writeDescriptorSets.push_back(writeDescriptorSet);

    context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
}

void SimpleRasterizerWindowEx::createCommandBuffers() {
    compute.commandBuffers.resize(swapchain->imageCount);
    for (int i = 0; i < swapchain->imageCount; ++i) {
        compute.commandBuffers[i] = context->getDevice()->getVkDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(compute.commandPools[i], vk::CommandBufferLevel::ePrimary, 1))[0];
    }

    compute.semaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
}

void SimpleRasterizerWindowEx::createComputeImage() {
    vk::ImageCreateInfo imageCreateInfo{
            {},
            vk::ImageType::e2D,
            vk::Format::eB8G8R8A8Unorm,
            {1000, 1000, 1},
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
                   reinterpret_cast<VkImage *>(&compute.image),
                   &compute.imageAllocation,
                   nullptr);

    context->executeTransient([&image = compute.image](vk::CommandBuffer commandBuffer) {
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
    compute.imageSampler = context->getDevice()->getVkDevice().createSampler(samplerCreateInfo);

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
    imageViewCreateInfo.image = compute.image;
    compute.imageView = context->getDevice()->getVkDevice().createImageView(imageViewCreateInfo);


}

void SimpleRasterizerWindowEx::createRenderPass() {

    vk::AttachmentDescription attachmentDescription({}, swapchain->format.format,
                                                    vk::SampleCountFlagBits::e1,
                                                    vk::AttachmentLoadOp::eDontCare,
                                                    vk::AttachmentStoreOp::eStore,
                                                    vk::AttachmentLoadOp::eDontCare,
                                                    vk::AttachmentStoreOp::eDontCare,
                                                    vk::ImageLayout::eUndefined,
                                                    vk::ImageLayout::ePresentSrcKHR);


    vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference);

    vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, {},
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     {}, vk::AccessFlagBits::eColorAttachmentWrite, {});

    vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescription, subpassDescription, dependency);

    graphics.renderPass = context->getDevice()->getVkDevice().createRenderPass(renderPassCreateInfo);
}


void SimpleRasterizerWindowEx::createRasterizer() {
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.flags = {};
    inputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = false;

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFront;
    rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationStateCreateInfo.flags = {};
    rasterizationStateCreateInfo.depthClampEnable = false;
    rasterizationStateCreateInfo.lineWidth = 1.0f;

    vk::PipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;
    blendAttachmentState.blendEnable = false;

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &blendAttachmentState;

    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
    depthStencilStateCreateInfo.depthTestEnable = false;
    depthStencilStateCreateInfo.depthWriteEnable = false;
    depthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;
    depthStencilStateCreateInfo.back.compareOp = vk::CompareOp::eAlways;

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

    std::vector<vk::DynamicState> dynamicStateEnables = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{{}, dynamicStateEnables};

    // Load shaders

    // Fragment shader
    std::ifstream fragmentFile("basic.frag.spv", std::ios::ate | std::ios::binary);
    if (!fragmentFile.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    size_t fileSize = (size_t) fragmentFile.tellg();
    std::vector<char> buffer(fileSize);
    fragmentFile.seekg(0);
    fragmentFile.read(buffer.data(), fileSize);
    fragmentFile.close();
    vk::ShaderModule fragmentShaderModule = context->getDevice()->getVkDevice().createShaderModule(
            vk::ShaderModuleCreateInfo{
                    {}, buffer.size(), reinterpret_cast<const uint32_t *>(buffer.data())
            });

    // Vertex shader
    std::ifstream vertexFile("basic.vert.spv", std::ios::ate | std::ios::binary);
    if (!vertexFile.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    fileSize = (size_t) vertexFile.tellg();
    buffer = std::vector<char>(fileSize);
    vertexFile.seekg(0);
    vertexFile.read(buffer.data(), fileSize);
    vertexFile.close();
    vk::ShaderModule vertexShaderModule = context->getDevice()->getVkDevice().createShaderModule(
            vk::ShaderModuleCreateInfo{
                    {}, buffer.size(), reinterpret_cast<const uint32_t *>(buffer.data())
            });

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

    shaderStages.at(0) = vk::PipelineShaderStageCreateInfo{
            {}, vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main"
    };
    shaderStages.at(1) = vk::PipelineShaderStageCreateInfo{
            {}, vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main"
    };


    vk::PipelineVertexInputStateCreateInfo inputStateCreateInfo{};
    inputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    inputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
    inputStateCreateInfo.vertexBindingDescriptionCount = 0;
    inputStateCreateInfo.pVertexBindingDescriptions = nullptr;

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.layout = graphics.pipelineLayout;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = nullptr;

    pipelineCreateInfo.pVertexInputState = &inputStateCreateInfo;

    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.stageCount = shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.renderPass = graphics.renderPass;

    graphics.pipeline = context->getDevice()->getVkDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo).value;

    context->getDevice()->getVkDevice().destroyShaderModule(fragmentShaderModule);
    context->getDevice()->getVkDevice().destroyShaderModule(vertexShaderModule);
}

void SimpleRasterizerWindowEx::createFramebuffers() {
    for (size_t i = 0; i < swapchain->imageViews.size(); i++) {
        vk::ImageView attachments[] = {swapchain->imageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = graphics.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain->extent.width;
        framebufferInfo.height = swapchain->extent.height;
        framebufferInfo.layers = 1;

        graphics.framebuffers.push_back(context->getDevice()->getVkDevice().createFramebuffer(framebufferInfo));
    }
}

void SimpleRasterizerWindowEx::createGraphicsDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding layoutBinding{
            0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment
    };

    vk::DescriptorSetLayoutCreateInfo descriptorLayout{{}, layoutBinding};

    graphics.descriptorSetLayout = context->getDevice()->getVkDevice().createDescriptorSetLayout(descriptorLayout);

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, graphics.descriptorSetLayout};

    graphics.pipelineLayout = context->getDevice()->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
}

void SimpleRasterizerWindowEx::createGraphicsDescriptorSet() {
    vk::DescriptorSetAllocateInfo allocateInfo{compute.descriptorPool, graphics.descriptorSetLayout};
    graphics.descriptorSet = context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo).at(0);

    vk::DescriptorImageInfo descriptorImageInfo(compute.imageSampler, compute.imageView, vk::ImageLayout::eGeneral);

    vk::WriteDescriptorSet writeDescriptorSet(graphics.descriptorSet,
                                              0,
                                              {},
                                              vk::DescriptorType::eCombinedImageSampler,
                                              descriptorImageInfo);

    context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
}


