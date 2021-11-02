#include "graphics/vulkan/core/Texture.h"

namespace vulkan {
    Texture::Texture(std::shared_ptr<Context> context,
                     vk::ImageCreateInfo imageCreateInfo,
                     VmaAllocationCreateInfo allocationInfo) : context(context) {

        vmaCreateImage(context->getAllocator(),
                       reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
                       &allocationInfo,
                       reinterpret_cast<VkImage *>(&image),
                       &allocation,
                       nullptr);




    }
}
