#include "examples/TestImWindow.h"

TestImWindow::TestImWindow() = default;

void TestImWindow::onImGuiFrameRender() {
    ImGui::Begin("Test");
    ImGui::Text("Test Pane");
    ImGui::End();
}

