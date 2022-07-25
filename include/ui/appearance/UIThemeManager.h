#ifndef SPECTRALGRAPHER_UITHEMEMANAGER_H
#define SPECTRALGRAPHER_UITHEMEMANAGER_H

#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include <imgui.h>

enum class ThemeLocation {
    ImGui,
    Memory,
    Filesystem
};

class Theme {
public:
    Theme(ThemeLocation location, std::string path) : location(location), path(std::move(path)) {}

    ThemeLocation location;
    std::string path;
};

class UIThemeManager {
public:
    UIThemeManager();

    ~UIThemeManager();

    void applyTheme(const Theme &theme);

    std::vector<Theme> listAvailableThemes();

    Theme getCurrentTheme();

private:
    Theme currentTheme;

    std::string loadSourceFromMemory(const std::string &path);

    void applyColors(nlohmann::json jsonColors, ImGuiStyle &style);

    void applySizes(nlohmann::json jsonSizes, ImGuiStyle &style);

};


#endif //SPECTRALGRAPHER_UITHEMEMANAGER_H
