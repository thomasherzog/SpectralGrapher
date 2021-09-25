#include "examples/TestImWindow.h"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(fonts);

TestImWindow::TestImWindow() {
    implotContext = ImPlot::CreateContext();

    auto fs = cmrc::fonts::get_filesystem();
    auto font = fs.open("fonts/verdana.ttf");
    std::string fontMem{font.begin(), font.end()};
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(fontMem.data(), fontMem.size(), 13.0f);

    context->executeTransient([](VkCommandBuffer commandBuffer) {
        return ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

TestImWindow::~TestImWindow() {
    ImPlot::DestroyContext(implotContext);
}

void TestImWindow::onImGuiFrameRender() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Settings")) {
            }
            if (ImGui::MenuItem("Exit")) {
                exit(0);
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
        ImGui::EndMainMenuBar();
    }

    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
}


