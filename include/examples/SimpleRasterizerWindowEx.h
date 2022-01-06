#ifndef SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
#define SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H

#include "windowing/VulkanWindow.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/SwapchainImageRenderer.h"
#include "renderer/ImGuiRenderer.h"

#ifndef _WIN32
#include "native/windows/Win32CustomTitlebar.h"
#endif

class SimpleRasterizerWindowEx : public windowing::VulkanWindow {
public:
    SimpleRasterizerWindowEx();

    ~SimpleRasterizerWindowEx();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onSwapchainRebuild() override;

    void initImGuiStyle();

    void createDockingSpace();

private:
    std::unique_ptr<ComputeRenderer> computeRenderer;

    std::unique_ptr<SwapchainImageRenderer> imageRenderer;

    std::unique_ptr<ImGuiRenderer> imguiRenderer;

    vk::Semaphore imgToImGuiSemaphore;

    vk::Semaphore computeToSwapchainSemaphore;

#ifndef _WIN32
    Win32CustomTitlebar titlebar;
#endif

    ImTextureID imguiTexture;
};


#endif //SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
