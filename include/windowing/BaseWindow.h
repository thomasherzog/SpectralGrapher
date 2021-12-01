#ifndef SPECTRALGRAPHER_BASEWINDOW_H
#define SPECTRALGRAPHER_BASEWINDOW_H

#include <vector>
#include <string>
#include <tuple>
#include <GLFW/glfw3.h>
#include <memory>
#include "native/Win32CustomTitlebar.h"

namespace windowing {
    class BaseWindow;
}

class windowing::BaseWindow {
public:
    BaseWindow(const std::string &title, const std::vector<std::tuple<int, int>> &windowHints);

    void renderWindow();

    GLFWwindow *getWindow();

protected:
    GLFWwindow *window;

    bool firstRender{true};

    virtual void onWindowRender() = 0;

    std::unique_ptr<Win32CustomTitlebar> titlebar;

};

#endif //SPECTRALGRAPHER_BASEWINDOW_H
