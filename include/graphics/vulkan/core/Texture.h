#ifndef SPECTRALGRAPHER_TEXTURE_H
#define SPECTRALGRAPHER_TEXTURE_H

#include <memory>
#include "graphics/vulkan/core/Context.h"

namespace vulkan {
    class Texture;

    //class Texture2D;
}

class vulkan::Texture {
public:
    Texture(std::shared_ptr<Context> context,
            vk::ImageCreateInfo imageCreateInfo,
            VmaAllocationCreateInfo allocationInfo);

    ~Texture();

    std::shared_ptr<Context> context;

    vk::Image image;

    vk::ImageLayout imageLayout;

    VmaAllocation allocation;

    vk::Sampler sampler;

    vk::DescriptorImageInfo descriptorImageInfo;


};

/*class vulkan::Texture2D {
public:
    Texture2D();

};*/


#endif //SPECTRALGRAPHER_TEXTURE_H
