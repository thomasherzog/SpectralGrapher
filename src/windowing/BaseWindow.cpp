#include "windowing/BaseWindow.h"

namespace windowing {

    BaseWindow::BaseWindow(const std::string &title, const std::vector<std::tuple<int, int>> &windowHints) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, false);
        for (auto hint : windowHints) {
            glfwWindowHint(std::get<0>(hint), std::get<1>(hint));
        }
        window = glfwCreateWindow(1920, 1080, title.c_str(), nullptr, nullptr);
    }

    void BaseWindow::renderWindow() {
        onWindowRender();
        if (firstRender) {
            glfwShowWindow(window);
            firstRender = false;
        }
    }

    GLFWwindow *BaseWindow::getWindow() {
        return window;
    }

}