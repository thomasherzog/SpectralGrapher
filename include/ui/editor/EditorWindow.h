#ifndef SPECTRALGRAPHER_EDITORWINDOW_H
#define SPECTRALGRAPHER_EDITORWINDOW_H

#include "windowing/VulkanWindow.h"
#include "renderer/general/SwapchainImageRenderer.h"
#include "renderer/general/ImGuiRenderer.h"

#include "ui/editor/popups/SettingsPopup.h"
#include "ui/editor/views/ViewportBarsView.h"
#include "ui/appearance/UIThemeManager.h"
#include "renderer/MandelbrotRenderer.h"

class EditorWindow : public windowing::VulkanWindow {
public:
    EditorWindow();

    ~EditorWindow();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onInitializeUI();

    void onRenderUI();

    void onSwapchainRebuild() override;

private:
    std::unique_ptr<ImGuiRenderer> imguiRenderer;

    std::unique_ptr<MandelbrotRenderer> computeRenderer;

    vk::Semaphore renderProcessToUISemaphore;

    std::unordered_map<std::string, std::tuple<std::unique_ptr<Popup>, std::function<void(Popup &popup)>>> popups;

    //TODO: Remove
    std::unique_ptr<ViewportBarsView> vbView;
    UIThemeManager tm;
};


#endif //SPECTRALGRAPHER_EDITORWINDOW_H
