#include "ui/editor/StartWindow.h"

#include "windowing/WindowManager.h"
#include "ui/editor/EditorWindow.h"

StartWindow::StartWindow() : windowing::VulkanWindow("Start Window", 900, 500) {
    imguiRenderer = std::make_unique<ImGuiRenderer>(context, window, *swapchain);
    tm.applyTheme(tm.listAvailableThemes().at(3));
    onInitializeUI();
}

StartWindow::~StartWindow() {
    context->getDevice()->getVkDevice().waitIdle();
}

void StartWindow::onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) {
    context->getDevice()->getVkDevice().resetFences(syncObject.fence);
    imguiRenderer->declareUserInterface([this]() { this->onRenderUI(); });
    std::vector<vk::PipelineStageFlags> pipelineStageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    std::vector<vk::Semaphore> semaphores = {syncObject.imageAvailableSemaphore};
    auto imguiRendererCmd = imguiRenderer->recordCommandBuffer(*swapchain, imageIndex);
    imguiRendererCmd.submit(
            semaphores,
            pipelineStageFlags,
            {syncObject.renderFinishedSemaphore},
            syncObject.fence
    );
}

void StartWindow::onInitializeUI() {

}

void StartWindow::onRenderUI() {
    ImGui::Begin("Test");

    if (ImGui::Button("DOING SOMETHING")) {
        auto manager = windowing::WindowManager::getInstance();
        manager->addWindow(std::make_shared<EditorWindow>());
        glfwSetWindowShouldClose(window, true);
        ImGui::SetCurrentContext(imguiRenderer->imguiContext);
    }

    ImGui::End();
}

void StartWindow::onSwapchainRebuild() {
    imguiRenderer->onSwapchainRebuild(*swapchain);
}


