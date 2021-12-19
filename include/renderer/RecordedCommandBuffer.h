#ifndef SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H
#define SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H

#include <utility>

#include "graphics/vulkan/core/Context.h"


class RecordedCommandBuffer {
public:
    RecordedCommandBuffer(vk::Queue queue, vk::CommandBuffer commandBuffer)
            : queue(queue), commandBuffer(commandBuffer) {

    }

    void submit(std::vector<vk::Semaphore> waitSemaphores,
                std::vector<vk::PipelineStageFlags> waitDstStageMask,
                std::vector<vk::Semaphore> signalSemaphore,
                vk::Fence fence) {

        vk::SubmitInfo submitInfo(
                waitSemaphores,
                waitDstStageMask,
                commandBuffer,
                signalSemaphore
        );
        queue.submit(submitInfo, fence);
    }

private:
    vk::Queue queue;

    vk::CommandBuffer commandBuffer;

};


#endif //SPECTRALGRAPHER_RECORDEDCOMMANDBUFFER_H
