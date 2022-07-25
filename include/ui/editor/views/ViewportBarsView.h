#ifndef SPECTRALGRAPHER_VIEWPORTBARSVIEW_H
#define SPECTRALGRAPHER_VIEWPORTBARSVIEW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/MandelbrotRenderer.h"

#ifdef _WIN32
#include "native/windows/Win32CustomTitlebar.h"
#endif

class ViewportBarsView {
public:
    explicit ViewportBarsView(GLFWwindow* window);

    ~ViewportBarsView();

    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);

    void renderView(std::unique_ptr<MandelbrotRenderer> const &mandelbrotRenderer);

private:
    GLFWwindow *window;

#ifdef _WIN32
    Win32CustomTitlebar titlebar;
#endif

};


#endif //SPECTRALGRAPHER_VIEWPORTBARSVIEW_H
