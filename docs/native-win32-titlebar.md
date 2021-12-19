# Win32 custom titlebar

## Load title bar icon font
```c++
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
```


## Style ImGui title bar
```c++
ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));
ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 11));
ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
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

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    if (ImGui::Button(u8"\uE949", ImVec2(50, 28))) {
        glfwIconifyWindow(window);
    }

    ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x - 50, 0));
    if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
        if (ImGui::Button(u8"\uE923", ImVec2(50, 28))) {
            glfwRestoreWindow(window);
        }
    } else {
        if (ImGui::Button(u8"\uE739", ImVec2(50, 28))) {
            glfwMaximizeWindow(window);
        }
    }

    ImGui::GetIO().Fonts->Fonts.Data[2];
    ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x - size.x, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(232,17,35).Value);
    if(ImGui::Button(u8"\uE106", ImVec2(50, 28))){
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
```