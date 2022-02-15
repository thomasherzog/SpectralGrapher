#ifndef SPECTRALGRAPHER_PROPERTIESVIEW_H
#define SPECTRALGRAPHER_PROPERTIESVIEW_H

#include "imgui.h"
#include "renderer/ComputeRenderer.h"

namespace PropertiesView {
    void renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer);
};


#endif //SPECTRALGRAPHER_PROPERTIESVIEW_H
