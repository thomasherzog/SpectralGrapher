#include "graphics/vulkan/core/SharedContext.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vulkan {

    SharedContext::SharedContext(const std::vector<std::tuple<std::string, bool>> &instanceExtensions,
                                 const std::vector<std::string> &validationLayers,
                                 const std::vector<std::tuple<std::string, bool>> &deviceExtensions,
                                 GLFWwindow *window) {

        vk::DynamicLoader dynamicLoader;
        auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        instance = std::make_shared<Instance>(instanceExtensions, validationLayers);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance->getInstance());

        debugMessenger = DebugMessenger(instance->getInstance());

        surface = std::make_shared<Surface>(instance->getInstance(), window);

        auto physicalDevice = PhysicalDevice::enumerateSupportedDevices(instance->getInstance(),
                                                                        deviceExtensions);

        device = std::make_shared<Device>(physicalDevice[0], surface->getSurface(), deviceExtensions);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(device->getVkDevice());
    }

    SharedContext::~SharedContext() {
        device.reset();
        surface->destroy(instance->getInstance());
        if (debugMessenger.has_value()) {
            debugMessenger->destroy(instance->getInstance());
            debugMessenger.reset();
        }
        instance.reset();
    }

    std::shared_ptr<Instance> SharedContext::getInstance() {
        return instance;
    }

    std::shared_ptr<Surface> SharedContext::getSurface() {
        return surface;
    }

    std::shared_ptr<Device> SharedContext::getDevice() {
        return device;
    }

}
