#ifndef SPECTRALGRAPHER_BASEWINDOW_H
#define SPECTRALGRAPHER_BASEWINDOW_H

#include <vector>
#include <string>
#include <tuple>
#include <GLFW/glfw3.h>
#include <memory>

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
};

#endif //SPECTRALGRAPHER_BASEWINDOW_H
