#ifndef SPECTRALGRAPHER_VULKANWINDOW_H
#define SPECTRALGRAPHER_VULKANWINDOW_H

#include "windowing/BaseWindow.h"

#include "graphics/vulkan/core/Swapchain.h"
#include "graphics/vulkan/core/InFlightFrames.h"

namespace windowing {
    class VulkanWindow;
}

class windowing::VulkanWindow : public BaseWindow {
public:
    VulkanWindow();

    ~VulkanWindow();

    virtual void onRender(vulkan::SyncObject syncObject, uint32_t imageIndex) = 0;

    virtual void onSwapchainRebuild() = 0;

    std::optional<uint32_t> acquireNextImage(vulkan::SyncObject syncObject);

    void presentFrame(vulkan::SyncObject syncObject, uint32_t imageIndex);

    void recreateSwapchain();

    void onWindowRender() override;

protected:
    std::shared_ptr<vulkan::Context> context;

    std::unique_ptr<vulkan::Swapchain> swapchain;

    std::vector<vk::CommandPool> commandPools;

    std::vector<vk::CommandBuffer> commandBuffers;

    std::unique_ptr<vulkan::InFlightFrames> inFlightFrames;

    std::vector<vk::Fence> imagesInFlight;

private:
    static std::vector<std::tuple<int, int>> getRequiredWindowHints();

};


#endif //SPECTRALGRAPHER_VULKANWINDOW_H
