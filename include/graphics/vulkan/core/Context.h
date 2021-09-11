#ifndef SPECTRALGRAPHER_CONTEXT_H
#define SPECTRALGRAPHER_CONTEXT_H

#include "graphics/vulkan/core/SharedContext.h"

namespace vulkan {
    class Context;
}

class vulkan::Context {
public:
    Context(const std::vector<std::tuple<std::string, bool>> &instanceExtensions,
            const std::vector<std::string> &validationLayers,
            const std::vector<std::tuple<std::string, bool>>& deviceExtensions,
            GLFWwindow *window);

    explicit Context(std::shared_ptr<SharedContext> sharedContext) ;

    ~Context();

    template<typename F>
    auto executeTransient(F const &commands) {
        vk::CommandBuffer commandBuffer = beginTransientExecution();
        auto result = commands(commandBuffer);
        endTransientExecution(commandBuffer);
        return result;
    }

    std::shared_ptr<Instance> getInstance();

    std::shared_ptr<Surface> getSurface();

    std::shared_ptr<Device> getDevice();

private:
    std::shared_ptr<SharedContext> sharedContext;

    vk::CommandPool transientCommandPool;

    vk::CommandBuffer beginTransientExecution();

    void endTransientExecution(vk::CommandBuffer commandBuffer);

};


#endif //SPECTRALGRAPHER_CONTEXT_H
