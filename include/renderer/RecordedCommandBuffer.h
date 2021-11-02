#ifndef SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H
#define SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H

#include <utility>

#include "graphics/vulkan/core/Context.h"


class RecordedCommandBuffer {
public:
    RecordedCommandBuffer(std::shared_ptr<vulkan::Context> context, vk::CommandBuffer commandBuffer)
            : context(std::move(context)), commandBuffer(commandBuffer) {

    }

    void submit(std::vector<vk::Semaphore> waitSemaphores,
                std::vector<vk::PipelineStageFlags> waitDstStageMask,
                vk::Semaphore signalSemaphore,
                vk::Fence fence) {

        vk::SubmitInfo submitInfo(
                waitSemaphores, waitDstStageMask,
                commandBuffer,
                signalSemaphore
        );
        context->getDevice()->getGraphicsQueue().submit(submitInfo, fence);
    }

private:
    std::shared_ptr<vulkan::Context> context;

    vk::CommandBuffer commandBuffer;

};



#endif //SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H
