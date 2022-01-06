#include "shape/ShapeManager.h"

ShapeManager::ShapeManager(std::shared_ptr<vulkan::Context> context) : context(std::move(context)) {}

ShapeManager::~ShapeManager() {
    for (auto const&[type, tuple]: shapes) {
        if (std::get<0>(tuple).buffer) {
            vmaDestroyBuffer(context->getAllocator(), std::get<0>(tuple).buffer, std::get<0>(tuple).allocation);
        }
    }
}

void ShapeManager::registerDefaultShapes() {
    registerShape("ch.herzog.BasicShapes", "Sphere",
                  "return length(point - object.position) - object.radius;",
                  {
                          {std::string("radius"),   ShapeFieldType::FLOAT},
                          {std::string("position"), ShapeFieldType::FLOAT3}
                  });

    registerShape("ch.herzog.Fractals", "Mandelbulb",
                  R"(
    vec3 z = point;
    float r = 0.0f;
    float dr = 1.0f;

    for(int i = 0; i < 20; i++){
        r = length(z);
        if(r > 2){
            break;
        }

        float theta = acos(z.z / r);
        float phi = atan(z.y / z.x);
        float zr = pow(r, object.power - 1);

        dr = object.power * zr * dr + 1.0f;

        theta = theta * object.power;
        phi = phi * object.power;

        z = vec3(r*sin(theta) * cos(phi), r*sin(theta) * sin(phi), r*cos(theta));
        z = z * zr + point;
    }

    return 0.5f * log(r) * r / dr;
 )",
                  {
                          {std::string("power"), ShapeFieldType::FLOAT}
                  });
}

ShapeType ShapeManager::registerShape(std::string pluginId, std::string name, std::string sourceCode,
                                      std::unordered_map<std::string, ShapeFieldType> fields) {
    ShapeType type(std::move(pluginId), std::move(name), std::move(sourceCode), std::move(fields));
    auto buffer = allocateShapeBuffer(type);
    shapes[type] = {buffer, {}, {}};
    return type;
}

void ShapeManager::addShape(const ShapeType &type, const ShapeContainer &container) {
    std::get<1>(shapes[type]).push_back(container);
    std::get<2>(shapes[type]) = BufferStatusType::RESIZED;
}

void ShapeManager::refreshShapeBuffers(vk::Queue queue) {
    for (auto &[type, properties]: shapes) {
        switch (static_cast<BufferStatusType::Value>(std::get<2>(properties))) {
            case BufferStatusType::RESIZED:
                queue.waitIdle();
                recreateShapeBuffer(type);
            case BufferStatusType::UPDATED:
                fillShapeBuffer(type);
                std::get<2>(properties) = BufferStatusType::NONE;
                break;
            case BufferStatusType::DEACTIVATED:
            case BufferStatusType::NONE:
                break;
        }
    }
}

std::string ShapeManager::generateShader() {
    std::string source;
    int i = 1;
    for (auto const&[type, containers]: shapes) {
        source += type.generateGLSLStruct() + "\n";
        source += type.generateGLSLDistanceFunction() + "\n";
        source += type.generateGLSLNormalFunction() + "\n";
        source += type.generateGLSLBufferDescription(1, i) + "\n";
        i++;
    }
    source += generateDistanceSwitch() + "\n";
    source += generateNormalSwitch();
    return source;
}


void ShapeManager::destroyShapeBuffer(const ShapeType &type) {
    if (std::get<0>(shapes[type]).buffer) {
        vmaDestroyBuffer(context->getAllocator(), std::get<0>(shapes[type]).buffer,
                         std::get<0>(shapes[type]).allocation);
    }
}

void ShapeManager::recreateShapeBuffer(const ShapeType &type) {
    destroyShapeBuffer(type);
    std::get<0>(shapes[type]) = allocateShapeBuffer(type);
}

void ShapeManager::fillShapeBuffer(const ShapeType &type) {
    void *objectData;
    vmaMapMemory(context->getAllocator(), std::get<0>(shapes[type]).allocation, &objectData);

    int offset = 0;
    for (auto &container: std::get<1>(shapes[type])) {
        for (auto const&[name, fieldType]: type.fields) {
            std::memcpy((void *) ((char *) objectData + offset), &container.fields[name],
                        fieldType.getAlignedMemorySize());
            offset += fieldType.getAlignedMemorySize();
        }
    }

    vmaUnmapMemory(context->getAllocator(), std::get<0>(shapes[type]).allocation);
}

AllocatedBuffer ShapeManager::allocateShapeBuffer(const ShapeType &type) {
    uint32_t size = 0;
    if (shapes.find(type) != shapes.end()) {
        size = std::get<1>(shapes[type]).size() * type.getAlignedMemorySize();
    }

    AllocatedBuffer buffer;
    vk::BufferCreateInfo createInfo = {
            {},
            size,
            vk::BufferUsageFlagBits::eStorageBuffer,
            {}, nullptr
    };

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    vmaCreateBuffer(
            context->getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo *>(&createInfo),
            &allocationInfo,
            reinterpret_cast<VkBuffer *>(&buffer.buffer),
            &buffer.allocation,
            nullptr
    );
    return buffer;
}


std::string ShapeManager::generateDistanceSwitch() {
    std::string source = "float getDistance(vec3 point, SignedDistanceField sdf) {\n";
    source += "switch(sdf.type) {\n";

    int i = 1;
    for (auto const&[type, containers]: shapes) {
        source += "case " + std::to_string(i) + ":\n";
        source += "return getDistance" + type.getUniqueName() + "(point, " + type.getUniqueName() +
                  "Buffer.objects[sdf.id]);\n";
        i++;
    }
    source += "}}";
    return source;
}

std::string ShapeManager::generateNormalSwitch() {
    std::string source = "vec3 getNormal(vec3 point, SignedDistanceField sdf) {\n";
    source += "switch(sdf.type) {\n";

    int i = 1;
    for (auto const&[type, containers]: shapes) {
        source += "case " + std::to_string(i) + ":\n";
        source += "return getNormal" + type.getUniqueName() + "(point, " + type.getUniqueName() +
                  "Buffer.objects[sdf.id]);\n";
        i++;
    }
    source += "}}";
    return source;
}