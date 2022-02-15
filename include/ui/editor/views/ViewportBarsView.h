#ifndef SPECTRALGRAPHER_VIEWPORTBARSVIEW_H
#define SPECTRALGRAPHER_VIEWPORTBARSVIEW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/ComputeRenderer.h"

#ifdef _WIN32
#include "native/windows/Win32CustomTitlebar.h"
#endif

class ViewportBarsView {
public:
    ViewportBarsView(GLFWwindow* window);

    ~ViewportBarsView();

    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);

private:
    GLFWwindow *window;

#ifdef _WIN32
    Win32CustomTitlebar titlebar;
#endif

};


#endif //SPECTRALGRAPHER_VIEWPORTBARSVIEW_H
