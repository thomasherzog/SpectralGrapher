#include "ui/editor/views/ViewportBarsView.h"

#ifdef _WIN32
ViewportBarsView::ViewportBarsView(GLFWwindow *window) : window(window), titlebar(window, {}) {

}
#else
ViewportBarsView::ViewportBarsView(GLFWwindow *window) : window(window) {}
#endif


ViewportBarsView::~ViewportBarsView() = default;

void ViewportBarsView::renderView(const std::unique_ptr<ComputeRenderer> &computeRenderer) {
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
                //isSettingsOpened = true;
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
    if (ImGui::BeginViewportSideBar("##SecondTitleBar", nullptr, ImGuiDir_Up, 22,
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("FPS: %.0f \t Samples: %i", ImGui::GetIO().Framerate, computeRenderer->ubo.sampleIndex);
            ImGui::SameLine();
            ImGui::Button("Test");
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
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
}

void ViewportBarsView::renderView(const std::unique_ptr<MandelbrotRenderer> &mandelbrotRenderer) {
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
                //isSettingsOpened = true;
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
    if (ImGui::BeginViewportSideBar("##SecondTitleBar", nullptr, ImGuiDir_Up, 22,
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
            ImGui::SameLine();
            ImGui::Button("Test");
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
    if (ImGui::BeginViewportSideBar("##MainStatusBar", nullptr, ImGuiDir_Down, 22,
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
