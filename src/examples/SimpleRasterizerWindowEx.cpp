#include "examples/SimpleRasterizerWindowEx.h"
#include <fstream>
#include <imgui_internal.h>

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

    auto font2 = fs.open("fonts/segoe-mdl2-assets.ttf");
    std::string fontMem2{font2.begin(), font2.end()};
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    ImFontConfig cfg;
    cfg.MergeMode = true;

    builder.AddChar(0xE949);
    builder.AddChar(0xE739);
    builder.AddChar(0xE923);
    builder.AddChar(0xE106);
    builder.BuildRanges(&ranges);
    auto winFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem2.data(), fontMem2.size(), 10.0f, &cfg,
                                                              ranges.Data);

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

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 11));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        if (ImGui::BeginMainMenuBar()) {

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Button("X", ImVec2(30, 28));

            if (ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::EndMenu();
            }

            ImVec2 size = ImVec2(50, 0);
            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 100, 0));

            ImGui::Button(u8"\uE949", ImVec2(50, 28));

            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 50, 0));
            if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
                ImGui::Button(u8"\uE923", ImVec2(50, 28));
            } else {
                ImGui::Button(u8"\uE739", ImVec2(50, 28));
            }

            ImGui::GetIO().Fonts->Fonts.Data[2];
            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x, 0));
            ImGui::Button(u8"\uE106", ImVec2(50, 28));

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar(3);


        if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", nullptr, ImGuiDir_Up, 30, 0)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("Happy secondary menu bar");
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }

        if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, 30, 0)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("Happy status bar");
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }


        ImGui::Begin("Das ist der Titel des Windows");

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

        ImGui::ShowDemoWindow();
    });

    auto imguiRendererCmd = imguiRenderer->recordCommandBuffer(*swapchain, imageIndex);
    imguiRendererCmd.submit(
            semaphores,
            pipelineStageFlags,
            syncObject.renderFinishedSemaphore,
            syncObject.fence
    );
}