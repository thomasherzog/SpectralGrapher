#ifndef SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
#define SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H

#include "windowing/VulkanWindow.h"

class SimpleRasterizerWindowEx : public windowing::VulkanWindow {
public:
    SimpleRasterizerWindowEx();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;
};


#endif //SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
