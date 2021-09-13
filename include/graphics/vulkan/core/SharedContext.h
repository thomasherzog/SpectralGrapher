#ifndef SPECTRALGRAPHER_SHAREDCONTEXT_H
#define SPECTRALGRAPHER_SHAREDCONTEXT_H

#include <optional>
#include <vulkan/vulkan.hpp>

#include <graphics/vulkan/core/Instance.h>
#include <graphics/vulkan/core/Surface.h>
#include <graphics/vulkan/core/Device.h>
#include <graphics/vulkan/debug/DebugMessenger.h>

#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

namespace vulkan {
    class SharedContext;
}

class vulkan::SharedContext {
public:
    SharedContext(const std::vector<std::tuple<std::string, bool>> &instanceExtensions,
                  const std::vector<std::string> &validationLayers,
                  const std::vector<std::tuple<std::string, bool>>& deviceExtensions,
                  GLFWwindow *window);

    ~SharedContext();

    std::shared_ptr<Instance> getInstance();

    std::shared_ptr<Surface> getSurface();

    std::shared_ptr<Device> getDevice();

    VmaAllocator getAllocator();

private:
    std::shared_ptr<Instance> instance;

    std::optional<DebugMessenger> debugMessenger;

    std::shared_ptr<Surface> surface;

    std::shared_ptr<Device> device;

    VmaAllocator allocator{};

};


#endif //SPECTRALGRAPHER_SHAREDCONTEXT_H
