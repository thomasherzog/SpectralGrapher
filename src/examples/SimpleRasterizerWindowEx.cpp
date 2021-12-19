#include "examples/SimpleRasterizerWindowEx.h"
#include <fstream>
#include <imgui_internal.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(fonts);

SimpleRasterizerWindowEx::SimpleRasterizerWindowEx()
#ifdef _WIN32
        : titlebar(window)
#endif
{
    computeRenderer = std::make_unique<ComputeRenderer>(context, swapchain->extent.width, swapchain->extent.height, 3);
    imageRenderer = std::make_unique<SwapchainImageRenderer>(context, *swapchain);
    imguiRenderer = std::make_unique<ImGuiRenderer>(context, window, *swapchain);

    imgToImGuiSemaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());
    computeToSwapchainSemaphore = context->getDevice()->getVkDevice().createSemaphore(vk::SemaphoreCreateInfo());

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

    builder.AddChar(0xE106); // "Close window" button icon
    builder.AddChar(0xE949); // "Iconify window" button icon
    builder.AddChar(0xE739); // "Maximize window" button icon
    builder.AddChar(0xE923); // "Restore window" button icon
    builder.BuildRanges(&ranges);
    auto winFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem2.data(), fontMem2.size(), 10.0f, &cfg,
                                                              ranges.Data);

    ImGui::GetIO().FontDefault = openSansFont;

    context->executeTransient([](VkCommandBuffer commandBuffer) {
        return ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    imguiTexture = ImGui_ImplVulkan_AddTexture(computeRenderer->accumulationImage.sampler,
                                               computeRenderer->accumulationImage.imageView,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

SimpleRasterizerWindowEx::~SimpleRasterizerWindowEx() {
    context->getDevice()->getVkDevice().waitIdle();
    context->getDevice()->getVkDevice().destroy(computeToSwapchainSemaphore);
    context->getDevice()->getVkDevice().destroy(imgToImGuiSemaphore);
}

void SimpleRasterizerWindowEx::onSwapchainRebuild() {
    computeRenderer->resizeImage(swapchain->extent.width, swapchain->extent.height);
    imageRenderer->onSwapchainResize(*swapchain);
    imguiRenderer->onSwapchainRebuild(*swapchain);
    imguiTexture = ImGui_ImplVulkan_AddTexture(computeRenderer->accumulationImage.sampler,
                                               computeRenderer->accumulationImage.imageView,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void SimpleRasterizerWindowEx::onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) {
    context->getDevice()->getVkDevice().resetFences(syncObject.fence);

    // ComputeRenderer
    auto computeCmd = computeRenderer->recordCommandBuffer();
    computeCmd.submit({}, {}, {computeToSwapchainSemaphore}, nullptr);

    // SwapchainImageRenderer
    std::vector<vk::PipelineStageFlags> pipelineStageFlags = {vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                                              vk::PipelineStageFlagBits::eComputeShader};
    std::vector<vk::Semaphore> semaphores = {syncObject.imageAvailableSemaphore, computeToSwapchainSemaphore};
    auto imageRendererCmd = imageRenderer->recordCommandBuffer(*swapchain, imageIndex,
                                                               computeRenderer->accumulationImage);
    imageRendererCmd.submit(semaphores, pipelineStageFlags, {imgToImGuiSemaphore}, nullptr);

    // ImGuiRenderer

    imguiRenderer->declareUserInterface([this]() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 11));

        if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        } else {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        float buttonHeight = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) ? 24 : 28;

        if (ImGui::BeginMainMenuBar()) {
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Button("X", ImVec2(30, buttonHeight));

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

            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
            if (ImGui::Button(u8"\uE949", ImVec2(50, buttonHeight))) {
                glfwIconifyWindow(window);
            }

            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 50, 0));
            if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
                if (ImGui::Button(u8"\uE923", ImVec2(50, buttonHeight))) {
                    glfwRestoreWindow(window);
                }
            } else {
                if (ImGui::Button(u8"\uE739", ImVec2(50, buttonHeight))) {
                    glfwMaximizeWindow(window);
                }
            }

            ImGui::GetIO().Fonts->Fonts.Data[2];
            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(232, 17, 35).Value);
            if (ImGui::Button(u8"\uE106", ImVec2(50, buttonHeight))) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::PopStyleColor(2);

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar(4);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", nullptr, ImGuiDir_Up, 30, 0)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("Secondary menu bar");
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }

        if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, 20, 0)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("Status bar");
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        ImGui::PopStyleVar();

        ImGui::Begin("Property inspector");

        if (ImGui::DragFloat3("Position", (float *) &computeRenderer->ubo.position, 0.1f)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::DragFloat3("Rotation", (float *) &computeRenderer->ubo.rotation, 0.1f)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::DragFloat("FOV", &computeRenderer->ubo.fov, 0.1f)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::InputInt("Samples", &computeRenderer->ubo.sampleCount)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::InputFloat("Epsilon", &computeRenderer->ubo.epsilon, 0.00005, 0.00005, "%.5f")) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::InputInt("Max time", &computeRenderer->ubo.maxTime)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::InputInt("Max steps", &computeRenderer->ubo.maxSteps)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::ColorEdit3("Background Color", (float *) &computeRenderer->ubo.backgroundColor)) {
            computeRenderer->ubo.sampleIndex = 0;
        }
        if (ImGui::InputFloat("Mandelbulb power", &computeRenderer->ubo.power, 0.1f)) {
            computeRenderer->ubo.sampleIndex = 0;
        }

        ImGui::End();

        ImGui::Begin("Settings");
        if (ImGui::CollapsingHeader("Devices")) {
            if (ImGui::BeginTable("Devices", 3)) {
                auto devices = context->getInstance()->getInstance().enumeratePhysicalDevices();
                ImGui::TableSetupColumn("Device name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Driver version", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("API version", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                for (auto &device: devices) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", device.getProperties().deviceName);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", device.getProperties().deviceType);
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", device.getProperties().apiVersion);
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader("Objects")) {
            if (ImGui::BeginTable("Objects", 3)) {
                auto devices = context->getInstance()->getInstance().enumeratePhysicalDevices();
                ImGui::TableSetupColumn("Position##col", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Radius##col", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                int i = 0;
                for (auto &sphere: computeRenderer->spheres) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(i);
                    if (ImGui::DragFloat3("Position", (float *) &sphere.position, 0.1f)) {
                        computeRenderer->ubo.sampleIndex = 0;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat("Radius", &sphere.radius, 0.1f)) {
                        computeRenderer->ubo.sampleIndex = 0;
                    }
                    ImGui::PopID();

                    i++;
                }

                for (auto &mandelbulb: computeRenderer->mandelbulbs) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(i);
                    if (ImGui::DragFloat3("Position", (float *) &mandelbulb.position, 0.1f)) {
                        computeRenderer->ubo.sampleIndex = 0;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::DragFloat("Power", &mandelbulb.power, 0.1f)) {
                        computeRenderer->ubo.sampleIndex = 0;
                    }
                    ImGui::PopID();

                    i++;
                }

                ImGui::EndTable();
            }
        }

        ImGui::Text("Samples rendered: %i", computeRenderer->ubo.sampleIndex);
        ImGui::End();

        ImGui::Begin("Viewport");
        ImGui::Image(imguiTexture, ImVec2(300, 300));
        ImGui::End();

        ImGui::ShowMetricsWindow();
    });

    pipelineStageFlags = {vk::PipelineStageFlagBits::eFragmentShader};
    semaphores = {imgToImGuiSemaphore};
    auto imguiRendererCmd = imguiRenderer->recordCommandBuffer(*swapchain, imageIndex);
    imguiRendererCmd.submit(
            semaphores,
            pipelineStageFlags,
            {syncObject.renderFinishedSemaphore},
            syncObject.fence
    );
}