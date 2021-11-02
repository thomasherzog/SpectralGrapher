#ifndef SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
#define SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H

#include "windowing/VulkanWindow.h"
#include "renderer/ComputeRenderer.h"
#include "renderer/SwapchainImageRenderer.h"
#include "renderer/ImGuiRenderer.h"

class SimpleRasterizerWindowEx : public windowing::VulkanWindow {
public:
    SimpleRasterizerWindowEx();

    ~SimpleRasterizerWindowEx();

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onSwapchainRebuild() override;

private:
    std::unique_ptr<ComputeRenderer> computeRenderer;

    std::unique_ptr<SwapchainImageRenderer> imageRenderer;

    std::unique_ptr<ImGuiRenderer> imguiRenderer;

    vk::Semaphore imgToImGuiSemaphore;

    std::vector<ImTextureID> textures;

};


#endif //SPECTRALGRAPHER_SIMPLERASTERIZERWINDOWEX_H
