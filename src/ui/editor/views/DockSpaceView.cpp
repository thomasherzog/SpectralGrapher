#include "ui/editor/views/DockSpaceView.h"

void DockSpaceView::renderView() {
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
