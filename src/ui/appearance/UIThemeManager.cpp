#include "ui/appearance/UIThemeManager.h"

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(themes);

#define jsonToImGuiSizeFloat(jsonArray, imguiStyle, name) \
    if ((jsonArray).contains(#name) && (jsonArray)[#name].is_number()) \
        (imguiStyle).name = (jsonArray)[#name].get<float>()

#define jsonToImGuiSizeVec2(jsonArray, imguiStyle, name) \
    if ((jsonArray).contains(#name) && (jsonArray)[#name].is_array() \
        && std::all_of((jsonArray).begin(), (jsonArray).end(), [](const nlohmann::json& el) { \
            return el.is_number(); \
        }) && (jsonArray)[#name].get<std::vector<float>>().size() == 2) \
        (imguiStyle).name = ImVec2((jsonArray)[#name][0], (jsonArray)[#name][1])

UIThemeManager::UIThemeManager() : currentTheme(listAvailableThemes().at(0)) {};

UIThemeManager::~UIThemeManager() = default;

void UIThemeManager::applyTheme(const Theme& theme) {
    ImGuiStyle style{};
    std::string themeSource;
    switch (theme.location) {
        case ThemeLocation::ImGui: {
            if (theme.path == "dark") {
                ImGui::StyleColorsDark(&style);
            } else if (theme.path == "light") {
                ImGui::StyleColorsLight(&style);
            } else if (theme.path == "classic") {
                ImGui::StyleColorsClassic(&style);
            }
            currentTheme = theme;
            ImGui::GetStyle() = style;
            return;
        }
        case ThemeLocation::Memory: {
            themeSource = loadSourceFromMemory(theme.path);
            break;
        }
        default: {
            return;
        }
    }
    auto jsonStruct = nlohmann::json::parse(themeSource);
    if (jsonStruct.contains("colors")) applyColors(jsonStruct["colors"], style);
    if (jsonStruct.contains("sizes")) applySizes(jsonStruct["sizes"], style);
    currentTheme = theme;
    ImGui::GetStyle() = style;
}

std::string UIThemeManager::loadSourceFromMemory(const std::string &name) {
    auto themeResource = cmrc::themes::get_filesystem().open("themes/" + name + ".json");
    return {themeResource.begin(), themeResource.end()};
}

std::vector<Theme> UIThemeManager::listAvailableThemes() {
    std::vector<Theme> names;

    names.emplace_back(ThemeLocation::ImGui, "dark");
    names.emplace_back(ThemeLocation::ImGui, "light");
    names.emplace_back(ThemeLocation::ImGui, "classic");

    auto directoryIterator = cmrc::themes::get_filesystem().iterate_directory("themes/");
    std::for_each(directoryIterator.begin(), directoryIterator.end(), [&names](auto const &element) {
        names.emplace_back(ThemeLocation::Memory, element.filename().substr(0, element.filename().find_last_of('.')));
    });

    return names;
}

void UIThemeManager::applyColors(nlohmann::json jsonColors, ImGuiStyle &style) {
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        auto imguiName = ImGui::GetStyleColorName(i);
        if (jsonColors.contains(imguiName) && jsonColors[imguiName].is_array()
            && jsonColors[imguiName].get<std::vector<float>>().size() == 4) {
            auto jsonColor = jsonColors[imguiName].get<std::vector<float>>();
            style.Colors[i] = ImVec4(jsonColor[0], jsonColor[1], jsonColor[2], jsonColor[3]);
        }
    }
}

void UIThemeManager::applySizes(nlohmann::json jsonSizes, ImGuiStyle &style) {
    jsonToImGuiSizeFloat(jsonSizes, style, Alpha);
    jsonToImGuiSizeFloat(jsonSizes, style, DisabledAlpha);
    jsonToImGuiSizeVec2(jsonSizes, style, WindowPadding);
    jsonToImGuiSizeFloat(jsonSizes, style, WindowRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, WindowBorderSize);
    jsonToImGuiSizeVec2(jsonSizes, style, WindowMinSize);
    jsonToImGuiSizeVec2(jsonSizes, style, WindowTitleAlign);
    jsonToImGuiSizeFloat(jsonSizes, style, ChildRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, ChildBorderSize);
    jsonToImGuiSizeFloat(jsonSizes, style, PopupRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, PopupBorderSize);
    jsonToImGuiSizeVec2(jsonSizes, style, FramePadding);
    jsonToImGuiSizeFloat(jsonSizes, style, FrameRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, FrameBorderSize);
    jsonToImGuiSizeVec2(jsonSizes, style, ItemSpacing);
    jsonToImGuiSizeVec2(jsonSizes, style, ItemInnerSpacing);
    jsonToImGuiSizeVec2(jsonSizes, style, CellPadding);
    jsonToImGuiSizeVec2(jsonSizes, style, TouchExtraPadding);
    jsonToImGuiSizeFloat(jsonSizes, style, IndentSpacing);
    jsonToImGuiSizeFloat(jsonSizes, style, ColumnsMinSpacing);
    jsonToImGuiSizeFloat(jsonSizes, style, ScrollbarSize);
    jsonToImGuiSizeFloat(jsonSizes, style, ScrollbarRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, GrabMinSize);
    jsonToImGuiSizeFloat(jsonSizes, style, GrabRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, LogSliderDeadzone);
    jsonToImGuiSizeFloat(jsonSizes, style, TabRounding);
    jsonToImGuiSizeFloat(jsonSizes, style, TabBorderSize);
    jsonToImGuiSizeFloat(jsonSizes, style, TabMinWidthForCloseButton);
    jsonToImGuiSizeVec2(jsonSizes, style, ButtonTextAlign);
    jsonToImGuiSizeVec2(jsonSizes, style, SelectableTextAlign);
    jsonToImGuiSizeVec2(jsonSizes, style, DisplayWindowPadding);
    jsonToImGuiSizeVec2(jsonSizes, style, DisplaySafeAreaPadding);
    jsonToImGuiSizeFloat(jsonSizes, style, MouseCursorScale);
    jsonToImGuiSizeFloat(jsonSizes, style, CurveTessellationTol);
    jsonToImGuiSizeFloat(jsonSizes, style, CircleTessellationMaxError);
    jsonToImGuiSizeFloat(jsonSizes, style, WindowShadowSize);
    jsonToImGuiSizeFloat(jsonSizes, style, WindowShadowOffsetDist);
    jsonToImGuiSizeFloat(jsonSizes, style, WindowShadowOffsetAngle);
}

Theme UIThemeManager::getCurrentTheme() {
    return currentTheme;
}
