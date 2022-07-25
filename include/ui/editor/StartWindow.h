#ifndef SPECTRALGRAPHER_STARTWINDOW_H
#define SPECTRALGRAPHER_STARTWINDOW_H

#include "windowing/VulkanWindow.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/general/SwapchainImageRenderer.h"
#include "renderer/general/ImGuiRenderer.h"

#include "ui/editor/popups/SettingsPopup.h"
#include "ui/editor/views/ViewportBarsView.h"
#include "ui/appearance/UIThemeManager.h"
#include "renderer/MandelbrotRenderer.h"

class StartWindow : public windowing::VulkanWindow  {
public:
    StartWindow();

    ~StartWindow();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onInitializeUI();

    void onRenderUI();

    void onSwapchainRebuild() override;

private:
    std::unique_ptr<ImGuiRenderer> imguiRenderer;

// REMOVE

    UIThemeManager tm;
};


#endif //SPECTRALGRAPHER_STARTWINDOW_H
