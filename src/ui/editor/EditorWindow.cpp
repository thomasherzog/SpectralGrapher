#include "ui/editor/EditorWindow.h"

#include "ui/editor/views/ViewportView.h"
#include "ui/editor/views/DockSpaceView.h"
#include "ui/editor/views/PropertiesView.h"
#include "ui/editor/views/ViewportBarsView.h"

#include "IconsMaterialDesign.h"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(fonts);

EditorWindow::EditorWindow() {
    imguiRenderer = std::make_unique<ImGuiRenderer>(context, window, *swapchain);
    computeRenderer = std::make_unique<MandelbrotRenderer>(context, swapchain->extent.width, swapchain->extent.height,
                                                           2);

    renderProcessToUISemaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());

    auto fs = cmrc::fonts::get_filesystem();
    auto font = fs.open("fonts/OpenSans-Regular.ttf");
    std::string fontMem{font.begin(), font.end()};
    auto openSansFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem.data(), fontMem.size(), 16.0f);

    // TODO: BEGIN INCLUDE ONLY ON WINDOWS (!)
    auto font2 = fs.open("fonts/segoe-mdl2-assets.ttf");
    std::string fontMem2{font2.begin(), font2.end()};
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    ImFontConfig cfg;
    cfg.MergeMode = true;

    builder.AddChar(0xE106); // "Close window" button icon
    builder.AddChar(0xE949); // "Iconify window" button icon
    builder.AddChar(0xE739); // "Maximize window" button icon
    builder.AddChar(0xE923); // "Restore window" button icon
    builder.BuildRanges(&ranges);
    auto winFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem2.data(), fontMem2.size(), 10.0f, &cfg,
                                                              ranges.Data);

    /*auto font2 = fs.open("fonts/MaterialIcons-Regular.ttf");
    std::string fontMem2 = {font2.begin(), font2.end()};
    static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;icons_config.GlyphOffset = { 0.f, 3.f };
    auto iconFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem2.data(), fontMem2.size(), 16.0f, &icons_config,
                                               icons_ranges);*/
    ImGui::GetIO().Fonts->Build();

    ImGui::GetIO().FontDefault = openSansFont;

    context->executeTransient([](VkCommandBuffer commandBuffer) {
        return ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    });
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    tm.applyTheme(tm.listAvailableThemes().at(3));
    // TODO: REMOVE (!)

    onInitializeUI();
}

EditorWindow::~EditorWindow() {
    context->getDevice()->getVkDevice().waitIdle();
    context->getDevice()->getVkDevice().destroy(renderProcessToUISemaphore);
};

void EditorWindow::onSwapchainRebuild() {
    imguiRenderer->onSwapchainRebuild(*swapchain);
    computeRenderer->resizeImage(swapchain->extent.width, swapchain->extent.height);
}

void EditorWindow::onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) {
    context->getDevice()->getVkDevice().resetFences(syncObject.fence);

    preRenderQueue.flush();

    auto computeCmd = computeRenderer->recordCommandBuffer();
    computeCmd.submit({}, {}, {renderProcessToUISemaphore}, nullptr);

    imguiRenderer->declareUserInterface([this]() { this->onRenderUI(); });

    std::vector<vk::PipelineStageFlags> pipelineStageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eComputeShader};
    std::vector<vk::Semaphore> semaphores = {syncObject.imageAvailableSemaphore, renderProcessToUISemaphore};
    auto imguiRendererCmd = imguiRenderer->recordCommandBuffer(*swapchain, imageIndex);
    imguiRendererCmd.submit(
            semaphores,
            pipelineStageFlags,
            {syncObject.renderFinishedSemaphore},
            syncObject.fence
    );
}

void EditorWindow::onInitializeUI() {
    popups.emplace("Settings", std::make_tuple(std::make_unique<SettingsPopup>(), [this](Popup &popup) {
        ((SettingsPopup &) popup).renderPopup(this->context);
    }));
    vbView = std::make_unique<ViewportBarsView>(window);
}

void EditorWindow::onRenderUI() {
    for (const auto &[name, value]: popups) {
        (std::get<1>(value))(*std::get<0>(value));
    }

    DockSpaceView::renderView();
    PropertiesView::renderView(computeRenderer);
    ViewportView::renderView(computeRenderer);
    vbView->renderView(computeRenderer);
}
