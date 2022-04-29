#ifndef SPECTRALGRAPHER_PROPERTIESVIEW_H
#define SPECTRALGRAPHER_PROPERTIESVIEW_H

#include "imgui.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/MandelbrotRenderer.h"

namespace PropertiesView {
    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);

    void renderView(std::unique_ptr<MandelbrotRenderer> const &mandelbrotRenderer);
};


#endif //SPECTRALGRAPHER_PROPERTIESVIEW_H
