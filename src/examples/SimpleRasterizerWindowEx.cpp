#include "examples/SimpleRasterizerWindowEx.h"
#include <fstream>
#include <imgui_internal.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(fonts);

SimpleRasterizerWindowEx::SimpleRasterizerWindowEx()
#ifndef _WIN32
        : titlebar(window)
#endif
{
    computeRenderer = std::make_unique<ComputeRenderer>(context, swapchain->extent.width, swapchain->extent.height, 2);
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

    builder.AddChar(0xE700); // "GlobalNavigationButton" button icon
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

    initImGuiStyle();
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
        createDockingSpace();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
        if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        } else {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        float buttonHeight = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) ? 24 : 28;

        if (ImGui::BeginMainMenuBar()) {

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8, 0.8, 0.8, 0.5));
            ImGui::SetCursorPos(
                    ImVec2((ImGui::GetIO().DisplaySize.x - ImGui::CalcTextSize("SpectralGrapher - Test Project").x) / 2,
                           0));
            ImGui::Text("SpectralGrapher - Test Project");
            ImGui::PopStyleColor();


            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Button(reinterpret_cast<const char *>(u8"\uE700"), ImVec2(30, buttonHeight));

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 11));
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Settings")) {
                    ImGui::OpenPopup("About");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, 1);
                }
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
            ImGui::PopStyleVar();

            ImVec2 size = ImVec2(50, 0);
            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 100, 0));

            if (ImGui::Button(reinterpret_cast<const char *>(u8"\uE949"), ImVec2(50, buttonHeight))) {
                glfwIconifyWindow(window);
            }

            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 50, 0));
            if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
                if (ImGui::Button(reinterpret_cast<const char *>(u8"\uE923"), ImVec2(50, buttonHeight))) {
                    glfwRestoreWindow(window);
                }
            } else {
                if (ImGui::Button(reinterpret_cast<const char *>(u8"\uE739"), ImVec2(50, buttonHeight))) {
                    glfwFocusWindow(window);
                    glfwMaximizeWindow(window);
                }
            }

            ImGui::GetIO().Fonts->Fonts.Data[2];
            ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(232, 17, 35).Value);
            if (ImGui::Button(reinterpret_cast<const char *>(u8"\uE106"), ImVec2(50, buttonHeight))) {
                glfwSetWindowShouldClose(window, true);
            }
            ImGui::PopStyleColor(2);

            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar(4);


        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, 22,
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                ImGui::Text("FPS: %.0f \t Samples: %i", ImGui::GetIO().Framerate, computeRenderer->ubo.sampleIndex);
                ImGui::EndMenuBar();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Properties");

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

        ImGui::End();

        ImGui::Begin("Settings X");
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

        ImGui::End();

        ImGui::ShowDemoWindow();

        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");

            auto[frameWidth, frameHeight] = ImGui::GetContentRegionAvail();
            ImGui::Image(imguiTexture, ImVec2(frameWidth, frameHeight));
            if (ImGui::IsItemHovered()) {
                if (ImGui::GetIO().MouseWheel != 0) {
                    auto rotationMatrix = glm::orientate3(computeRenderer->ubo.rotation);
                    auto directionVector = rotationMatrix * glm::vec3(1, 0, 0);
                    computeRenderer->ubo.position += directionVector * ImGui::GetIO().MouseWheel;
                    computeRenderer->ubo.sampleIndex = 0;
                    //TODO: Wrong
                }
            }

            //ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
            //ImGui::Text("Top left viewport position!");

            ImGui::End();
            ImGui::PopStyleVar();
        }
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

void SimpleRasterizerWindowEx::initImGuiStyle() {
    constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b) {
        return ImVec4((float) r / 255.0f, (float) g / 255.0f, (float) b / 255.0f, 1.0f);
    };

    auto &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
    const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
    const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

    const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
    const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
    const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

    const ImVec4 textColor = ColorFromBytes(255, 255, 255);
    const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
    const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;
    colors[ImGuiCol_FrameBg] = panelColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    colors[ImGuiCol_MenuBarBg] = panelColor;
    colors[ImGuiCol_ScrollbarBg] = panelColor;
    colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    colors[ImGuiCol_CheckMark] = panelActiveColor;
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelHoverColor;
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = borderColor;
    colors[ImGuiCol_SeparatorActive] = borderColor;
    colors[ImGuiCol_ResizeGrip] = bgColor;
    colors[ImGuiCol_ResizeGripHovered] = panelColor;
    colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    colors[ImGuiCol_PlotLines] = panelActiveColor;
    colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    colors[ImGuiCol_ModalWindowDimBg] = bgColor;
    colors[ImGuiCol_DragDropTarget] = bgColor;
    colors[ImGuiCol_NavHighlight] = bgColor;
    colors[ImGuiCol_DockingPreview] = panelActiveColor;
    colors[ImGuiCol_Tab] = bgColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = bgColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;

    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.32f, 0.32f, 0.35f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.31f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.58f);


    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 3);
    style.CellPadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 0;

    style.WindowRounding = 5;
    style.ChildRounding = 0;
    style.FrameRounding = 3;
    style.PopupRounding = 5;
    style.ScrollbarRounding = 2;
    style.GrabRounding = 2;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 3;

    style.WindowTitleAlign = ImVec2(0, 0.5);
    style.ButtonTextAlign = ImVec2(0.5, 0.5);
}

void SimpleRasterizerWindowEx::createDockingSpace() {
    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    auto windowFlags = ImGuiWindowFlags_NoDocking
                       | ImGuiWindowFlags_NoTitleBar
                       | ImGuiWindowFlags_NoCollapse
                       | ImGuiWindowFlags_NoResize
                       | ImGuiWindowFlags_NoMove
                       | ImGuiWindowFlags_NoBringToFrontOnFocus
                       | ImGuiWindowFlags_NoNavFocus
                       | ImGuiWindowFlags_NoBackground
                       | ImGuiWindowFlags_NoDecoration;

    auto dockFlags = ImGuiDockNodeFlags_PassthruCentralNode
                     | ImGuiDockNodeFlags_NoWindowMenuButton
                     | ImGuiDockNodeFlags_NoCloseButton;

    ImGui::Begin("Dock", nullptr, windowFlags);

    if (!ImGui::DockBuilderGetNode(ImGui::GetID("MainDockingSpace"))) {
        auto dockspace = ImGui::GetID("MainDockingSpace");
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace);

        auto sideBarRight = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.2f, nullptr, &dockspace);

        ImGui::DockBuilderDockWindow("Properties", sideBarRight);
        ImGui::DockBuilderDockWindow("Viewport", dockspace);

        ImGui::DockBuilderFinish(dockspace);
    }

    auto dockspaceId = ImGui::GetID("MainDockingSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockFlags);

    ImGui::End();
    ImGui::PopStyleVar(3);
}
