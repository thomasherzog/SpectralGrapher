#include "examples/SimpleRasterizerWindowEx.h"
#include <fstream>

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(fonts);

SimpleRasterizerWindowEx::SimpleRasterizerWindowEx() {
    computeRenderer = std::make_unique<ComputeRenderer>(context, swapchain->extent.width, swapchain->extent.height, 3);
    imageRenderer = std::make_unique<SwapchainImageRenderer>(context, *swapchain);
    imguiRenderer = std::make_unique<ImGuiRenderer>(context, window, *swapchain);
    imgToImGuiSemaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());

    auto fs = cmrc::fonts::get_filesystem();
    auto font = fs.open("fonts/OpenSans-Regular.ttf");
    std::string fontMem{font.begin(), font.end()};
    auto openSansFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem.data(), fontMem.size(), 16.0f);
    ImGui::GetIO().FontDefault = openSansFont;
    context->executeTransient([](VkCommandBuffer commandBuffer) {
        return ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

SimpleRasterizerWindowEx::~SimpleRasterizerWindowEx() {
    context->getDevice()->getVkDevice().waitIdle();
    context->getDevice()->getVkDevice().destroySemaphore(imgToImGuiSemaphore);
}

void SimpleRasterizerWindowEx::onSwapchainRebuild() {
    computeRenderer->resizeImage(swapchain->extent.width, swapchain->extent.height);
    imageRenderer->onSwapchainResize(*swapchain);
    imguiRenderer->onSwapchainRebuild(*swapchain);
}

void SimpleRasterizerWindowEx::onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) {
    context->getDevice()->getVkDevice().resetFences(syncObject.fence);

    // ComputeRenderer
    auto computeCmd = computeRenderer->recordCommandBuffer();
    computeCmd.submit({}, {}, computeRenderer->semaphore, nullptr);


    // SwapchainImageRenderer
    std::vector<vk::PipelineStageFlags> pipelineStageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eFragmentShader};
    std::vector<vk::Semaphore> semaphores = {syncObject.imageAvailableSemaphore, computeRenderer->semaphore};
    auto imageRendererCmd = imageRenderer->recordCommandBuffer(*swapchain,
                                                               imageIndex,
                                                               computeRenderer->images[computeRenderer->imageIndex]);
    imageRendererCmd.submit(semaphores,
                            pipelineStageFlags,
                            imgToImGuiSemaphore,
                            nullptr);


    // ImGuiRenderer
    pipelineStageFlags = {vk::PipelineStageFlagBits::eFragmentShader};
    semaphores = {imgToImGuiSemaphore};

    imguiRenderer->declareUserInterface([this, &imageIndex]() {
        ImGui::Begin("Das ist der Titel des Windows");

        ImGui::Image(ImGui::GetIO().Fonts->TexID, ImVec2(100, 100));
        if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0) {
            // TODO: wrong rotation
            float factor = ImGui::GetIO().MouseWheel * 0.5f;
            glm::vec3 camRot = computeRenderer->ubo.rotation;
            glm::mat3x3 eulerTransform = glm::mat3x3(glm::eulerAngleXYZ(camRot.x, -camRot.y, -camRot.z));
            glm::vec3 direction = glm::normalize(eulerTransform * glm::vec3(1, 0, 0));
            computeRenderer->ubo.position += factor * direction;
            computeRenderer->updateUniformBuffer();
        }

        ImGui::Text("Lorem ipsum dolor sit amet");

        if (ImGui::DragFloat3("Position", (float *) &computeRenderer->ubo.position, 0.1f)) {
            computeRenderer->updateUniformBuffer();
        }
        if (ImGui::DragFloat3("Rotation", (float *) &computeRenderer->ubo.rotation, 0.1f)) {
            computeRenderer->updateUniformBuffer();
        }
        if (ImGui::DragFloat("FOV", &computeRenderer->ubo.fov, 0.1f)) {
            computeRenderer->updateUniformBuffer();
        }

        ImGui::End();
    });

    auto imguiRendererCmd = imguiRenderer->recordCommandBuffer(*swapchain, imageIndex);
    imguiRendererCmd.submit(
            semaphores,
            pipelineStageFlags,
            syncObject.renderFinishedSemaphore,
            syncObject.fence
    );
}