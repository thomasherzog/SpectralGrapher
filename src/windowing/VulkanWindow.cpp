#include "windowing/VulkanWindow.h"

namespace windowing {

    VulkanWindow::VulkanWindow() : BaseWindow("This is a window title", getRequiredWindowHints()) {
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("Vulkan not supported!");
        }

        std::vector<std::tuple<std::string, bool>> instanceExtensions = {
                //std::make_tuple("TESTTEST", false), Extension not optional and not available. will crash.
                std::make_tuple("OOF", true) // Extension optional
        };
        const std::vector<std::string> validationLayers = {
                "VK_LAYER_KHRONOS_validation",
                //"VK_LAYER_LUNARG_standard_validation"
        };
        std::vector<std::tuple<std::string, bool>> deviceExtensions = {
                //std::make_tuple("TESTTEST", false), Extension not optional and not available. will crash.
                std::make_tuple(VK_KHR_SWAPCHAIN_EXTENSION_NAME, false),
                std::make_tuple("OOF", true) // Extension optional
        };

        uint32_t extensions_count = 0;
        const char **extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
        std::vector<const char *> glfwExtensions(extensions, extensions + extensions_count);
        for (auto ex: glfwExtensions) {
            instanceExtensions.emplace_back(ex, false);
        }

        context = std::make_shared<vulkan::Context>(instanceExtensions, validationLayers, deviceExtensions, window);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        uint32_t windowSize[2] = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        swapchain = std::make_unique<vulkan::Swapchain>(context, windowSize);

        inFlightFrames = std::make_unique<vulkan::InFlightFrames>(context, 2);

        imagesInFlight.resize(swapchain->imageCount, VK_NULL_HANDLE);

        for (int i = 0; i < swapchain->imageCount; ++i) {

            commandPools.emplace_back(
                    context->getDevice()->getVkDevice().createCommandPool(
                            vk::CommandPoolCreateInfo({}, context->getDevice()->getGraphicsQueueFamily())));

        }

        commandBuffers.resize(swapchain->imageCount);
        for (int i = 0; i < swapchain->imageCount; ++i) {
            commandBuffers[i] = context->getDevice()->getVkDevice().allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo(commandPools[i], vk::CommandBufferLevel::ePrimary, 1))[0];
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int, int) {
            static_cast<VulkanWindow *>(glfwGetWindowUserPointer(window))->onWindowRender();
        });

    }

    VulkanWindow::~VulkanWindow() {
        context->getDevice()->getVkDevice().waitIdle();

        for (auto commandPool: commandPools) {
            context->getDevice()->getVkDevice().destroyCommandPool(commandPool);
        }

        glfwDestroyWindow(window);
    }

    std::optional<uint32_t> VulkanWindow::acquireNextImage(vulkan::SyncObject syncObject) {
        context->getDevice()->getVkDevice().waitForFences(1, &syncObject.fence, true, UINT64_MAX);

        auto imageIndexResult = context->getDevice()->getVkDevice().acquireNextImageKHR(swapchain->swapchain,
                                                                                        UINT64_MAX,
                                                                                        syncObject.imageAvailableSemaphore,
                                                                                        nullptr);
        switch (imageIndexResult.result) {
            case vk::Result::eSuccess:
            case vk::Result::eSuboptimalKHR:
                break;
            case vk::Result::eErrorOutOfDateKHR:
                recreateSwapchain();
                return std::nullopt;
            default:
                throw std::runtime_error("Swap chain image acquisition failed");
        }
        auto imageIndex = imageIndexResult.value;

        if (imagesInFlight[imageIndex]) {
            context->getDevice()->getVkDevice().waitForFences(1, &imagesInFlight[imageIndex], true, UINT64_MAX);
        }

        imagesInFlight[imageIndex] = syncObject.fence;
        return imageIndex;
    }

    void VulkanWindow::presentFrame(vulkan::SyncObject syncObject, uint32_t imageIndex) {
        vk::PresentInfoKHR presentInfo(1, &syncObject.renderFinishedSemaphore,
                                       1, &swapchain->swapchain,
                                       &imageIndex);

        vk::Result result;
        try {
            result = context->getDevice()->getPresentQueue().presentKHR(presentInfo);
        } catch (vk::OutOfDateKHRError &) {
            result = vk::Result::eErrorOutOfDateKHR;
        }

        switch (result) {
            case vk::Result::eSuccess:
            case vk::Result::eSuboptimalKHR:
                break;
            case vk::Result::eErrorOutOfDateKHR:
                recreateSwapchain();
                break;
            default:
                throw std::runtime_error("Swap chain image presentation failed");
        }

    }

    void VulkanWindow::recreateSwapchain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width != 0 && height != 0) {
            context->getDevice()->getVkDevice().waitIdle();
            uint32_t dim[2] = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
            swapchain->rebuild(dim);
            imagesInFlight.resize(swapchain->imageCount, nullptr);
            onSwapchainRebuild();
        }
    }

    void VulkanWindow::onWindowRender() {
        auto syncObject = inFlightFrames->getNextSyncObject();
        std::optional<uint32_t> imageIndex = acquireNextImage(syncObject);
        if (imageIndex.has_value()) {
            onRender(syncObject, imageIndex.value());
            presentFrame(syncObject, imageIndex.value());
        }
    }

    std::vector<std::tuple<int, int>> VulkanWindow::getRequiredWindowHints() {
        std::vector<std::tuple<int, int>> hints;
        hints.emplace_back(GLFW_CLIENT_API, GLFW_NO_API);
        return hints;
    }

}

