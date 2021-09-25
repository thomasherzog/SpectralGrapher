#include "examples/TestImWindow.h"

TestImWindow::TestImWindow() = default;

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

}

