#ifndef SPECTRALGRAPHER_IMGUIWINDOW_H
#define SPECTRALGRAPHER_IMGUIWINDOW_H

#include "windowing/VulkanWindow.h"

#include "renderer/general/ImGuiRenderer.h"

namespace windowing {
    class ImGuiWindow;
}

class windowing::ImGuiWindow : public VulkanWindow {
public:
    ImGuiWindow();

    ~ImGuiWindow();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) final;

    void onSwapchainRebuild() final;

    virtual void onInitializeUI() = 0;

    virtual void onRenderUI() = 0;

private:
    std::unique_ptr<ImGuiRenderer> imguiRenderer;

};


#endif //SPECTRALGRAPHER_IMGUIWINDOW_H
