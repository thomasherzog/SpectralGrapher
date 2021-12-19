#include "renderer/SwapchainImageRenderer.h"

#include <fstream>
#include <utility>

SwapchainImageRenderer::SwapchainImageRenderer(std::shared_ptr<vulkan::Context> context,
                                               vulkan::Swapchain &swapchain) : context(std::move(context)) {
    createCommandPool(swapchain.imageCount);
    createCommandBuffers(swapchain.imageCount);
    createRenderPass(swapchain);
    createFramebuffers(swapchain);
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();
    createRasterizer();
}

SwapchainImageRenderer::~SwapchainImageRenderer() {
    for (int i = 0; i < commandPools.size(); ++i) {
        context->getDevice()->getVkDevice().freeCommandBuffers(commandPools[i], commandBuffers[i]);
        context->getDevice()->getVkDevice().destroyCommandPool(commandPools[i]);
    }
    for(auto &descriptorPool : descriptorPools) {
        context->getDevice()->getVkDevice().destroyDescriptorPool(descriptorPool);
    }
    context->getDevice()->getVkDevice().destroyPipeline(pipeline);
    context->getDevice()->getVkDevice().destroyPipelineLayout(pipelineLayout);
    context->getDevice()->getVkDevice().destroyDescriptorSetLayout(descriptorSetLayout);
    for (auto &framebuffer: framebuffers) {
        context->getDevice()->getVkDevice().destroyFramebuffer(framebuffer);
    }
    framebuffers.clear();
    context->getDevice()->getVkDevice().destroyRenderPass(renderPass);
}


RecordedCommandBuffer SwapchainImageRenderer::recordCommandBuffer(vulkan::Swapchain &swapchain, int imageIndex,
                                                                  AllocatedImage image) {
    context->getDevice()->getVkDevice().resetCommandPool(commandPools[imageIndex]);
    auto commandBuffer = commandBuffers[imageIndex];

    updateDescriptorSet(image, imageIndex);

    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = swapchain.extent.width;
    renderPassBeginInfo.renderArea.extent.height = swapchain.extent.height;
    renderPassBeginInfo.framebuffer = framebuffers[imageIndex];

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);


    vk::Viewport viewport{
            0, 0,
            static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height),
            0.0f, 1.0f
    };
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{
            {0,                      0},
            {swapchain.extent.width, swapchain.extent.height}
    };
    commandBuffer.setScissor(0, scissor);


    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets[imageIndex],
                                     nullptr);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    return {context->getDevice()->getGraphicsQueue(), commandBuffer};
}


void SwapchainImageRenderer::onSwapchainResize(vulkan::Swapchain &swapchain) {
    for (auto &framebuffer: framebuffers) {
        context->getDevice()->getVkDevice().destroyFramebuffer(framebuffer);
    }
    framebuffers.clear();
    context->getDevice()->getVkDevice().destroyRenderPass(renderPass);
    createRenderPass(swapchain);
    createFramebuffers(swapchain);
}

void SwapchainImageRenderer::updateDescriptorSet(AllocatedImage image, int imageIndex) {
    vk::DescriptorImageInfo descriptorImageInfo(image.sampler, image.imageView, vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet writeDescriptorSet(descriptorSets[imageIndex],
                                              0,
                                              {},
                                              vk::DescriptorType::eCombinedImageSampler,
                                              descriptorImageInfo);

    context->getDevice()->getVkDevice().updateDescriptorSets(writeDescriptorSet, nullptr);
}


void SwapchainImageRenderer::createCommandPool(int imageCount) {
    for (int i = 0; i < imageCount; ++i) {
        commandPools.emplace_back(
                context->getDevice()->getVkDevice().createCommandPool(
                        vk::CommandPoolCreateInfo({}, context->getDevice()->getGraphicsQueueFamily())));

    }
}

void SwapchainImageRenderer::createCommandBuffers(int imageCount) {
    commandBuffers.resize(imageCount);
    for (int i = 0; i < imageCount; ++i) {
        commandBuffers[i] = context->getDevice()->getVkDevice().allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(commandPools[i], vk::CommandBufferLevel::ePrimary, 1))[0];
    }
}

void SwapchainImageRenderer::createRenderPass(vulkan::Swapchain &swapchain) {
    vk::AttachmentDescription attachmentDescription({}, swapchain.format.format,
                                                    vk::SampleCountFlagBits::e1,
                                                    vk::AttachmentLoadOp::eDontCare,
                                                    vk::AttachmentStoreOp::eStore,
                                                    vk::AttachmentLoadOp::eDontCare,
                                                    vk::AttachmentStoreOp::eDontCare,
                                                    vk::ImageLayout::eUndefined,
                                                    vk::ImageLayout::eGeneral);

    vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpassDescription({}, vk::PipelineBindPoint::eGraphics, {}, attachmentReference);

    vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, {},
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                     {}, vk::AccessFlagBits::eColorAttachmentWrite, {});

    vk::RenderPassCreateInfo renderPassCreateInfo({}, attachmentDescription, subpassDescription, dependency);

    renderPass = context->getDevice()->getVkDevice().createRenderPass(renderPassCreateInfo);
}

void SwapchainImageRenderer::createFramebuffers(vulkan::Swapchain &swapchain) {
    for (size_t i = 0; i < swapchain.imageViews.size(); i++) {
        vk::ImageView attachments[] = {swapchain.imageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;

        framebuffers.push_back(context->getDevice()->getVkDevice().createFramebuffer(framebufferInfo));
    }
}

void SwapchainImageRenderer::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding layoutBinding{
            0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            vk::ShaderStageFlagBits::eFragment
    };

    vk::DescriptorSetLayoutCreateInfo descriptorLayout{{}, layoutBinding};

    descriptorSetLayout = context->getDevice()->getVkDevice().createDescriptorSetLayout(descriptorLayout);

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, descriptorSetLayout};

    pipelineLayout = context->getDevice()->getVkDevice().createPipelineLayout(pipelineLayoutCreateInfo);
}

void SwapchainImageRenderer::createDescriptorPool() {
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

void SwapchainImageRenderer::createDescriptorSets() {
    for (int i = 0; i < 3; i++) {
        vk::DescriptorSetAllocateInfo allocateInfo{descriptorPools[i], descriptorSetLayout};
        descriptorSets.push_back(context->getDevice()->getVkDevice().allocateDescriptorSets(allocateInfo).at(0));
    }
}


void SwapchainImageRenderer::createRasterizer() {
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
    pipelineCreateInfo.layout = pipelineLayout;
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
    pipelineCreateInfo.renderPass = renderPass;

    pipeline = context->getDevice()->getVkDevice().createGraphicsPipeline(nullptr, pipelineCreateInfo).value;

    context->getDevice()->getVkDevice().destroyShaderModule(fragmentShaderModule);
    context->getDevice()->getVkDevice().destroyShaderModule(vertexShaderModule);
}

