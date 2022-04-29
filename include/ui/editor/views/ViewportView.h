#ifndef SPECTRALGRAPHER_VIEWPORTVIEW_H
#define SPECTRALGRAPHER_VIEWPORTVIEW_H

#include "imgui.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/MandelbrotRenderer.h"

namespace ViewportView {
    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);

    void renderView(std::unique_ptr<MandelbrotRenderer> const &mandelbrotRenderer);
}

#endif //SPECTRALGRAPHER_VIEWPORTVIEW_H
