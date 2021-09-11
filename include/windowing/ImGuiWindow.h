#ifndef SPECTRALGRAPHER_IMGUIWINDOW_H
#define SPECTRALGRAPHER_IMGUIWINDOW_H

#include "windowing/VulkanWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace windowing {
    class ImGuiWindow;
}

class windowing::ImGuiWindow : public VulkanWindow {
public:
    ImGuiWindow();

    ~ImGuiWindow();

    virtual void onImGuiFrameRender() = 0;

    void renderImGuiFrame(vulkan::SyncObject syncObject, uint32_t imageIndex, ImDrawData *drawData);

    void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) override;

    void onSwapchainRebuild() override;

private:
    ImGuiContext *imguiContext;

    std::vector<vk::Framebuffer> framebuffers;

    vk::DescriptorPool descriptorPool;

    vk::RenderPass renderPass;

    void createRenderPass();

};


#endif //SPECTRALGRAPHER_IMGUIWINDOW_H
