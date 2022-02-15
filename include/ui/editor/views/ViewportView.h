#ifndef SPECTRALGRAPHER_VIEWPORTVIEW_H
#define SPECTRALGRAPHER_VIEWPORTVIEW_H

#include "imgui.h"
#include "renderer/ComputeRenderer.h"

namespace ViewportView {
    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);
}

#endif //SPECTRALGRAPHER_VIEWPORTVIEW_H
