#ifndef SPECTRALGRAPHER_SHAPEMANAGER_H
#define SPECTRALGRAPHER_SHAPEMANAGER_H

#include <utility>
#include <vector>
#include <variant>
#include <glm/vec3.hpp>

#include "shape/ShapeType.h"

#include "graphics/vulkan/core/Context.h"

struct SignedDistanceField {
    int type = -1;
    int id = -1;
};

class ManagerStatusType {
public:
    enum Value {
        PIPELINE_OUTDATED,
        NONE
    };
};

class BufferStatusType {
public:
    enum Value {
        RESIZED,
        UPDATED,
        DEACTIVATED,
        NONE
    };

    BufferStatusType() : value(Value::NONE) {}

    BufferStatusType(Value value) : value(value) {}

    constexpr explicit operator Value() const { return value; }

    operator bool() = delete;

    bool operator==(BufferStatusType type) const { return value == type.value; }

    bool operator!=(BufferStatusType type) const { return value != type.value; }

private:
    Value value;
};

class ShapeContainer {
public:

    explicit ShapeContainer(const ShapeType &shapeType) {

    }

    bool validate(const ShapeType &shapeType) {

    }

    std::unordered_map<std::string, std::variant<std::monostate, float, glm::vec3, SignedDistanceField>> fields;
};

struct AllocatedBuffer {
    VmaAllocation allocation;
    vk::Buffer buffer;
};

class ShapeManager {
public:
    explicit ShapeManager(std::shared_ptr<vulkan::Context> context);

    ~ShapeManager();

    // =================
    // Buffer management
    // =================

    void registerDefaultShapes();

    ShapeType registerShape(std::string pluginId, std::string name, std::string sourceCode,
                            std::unordered_map<std::string, ShapeFieldType> fields);

    void addShape(const ShapeType &type, const ShapeContainer &container);

    void refreshShapeBuffers(vk::Queue queue);

    // ==============================
    // Shader & descriptor management
    // ==============================

    std::vector<ShapeType> getBindingPositions() {
        std::vector<ShapeType> bindings{};
        for (auto const&[type, properties]: shapes) {
            if(std::get<0>(properties).buffer && std::get<2>(properties) != BufferStatusType::DEACTIVATED) {
                bindings.push_back(type);
            }
        }
        return bindings;
    }

    std::string generateDistanceSwitch();

    std::string generateNormalSwitch();

    std::string generateShader();

    vk::DescriptorSetLayout createDescriptorSetLayout() {

    }

    std::unordered_map<ShapeType, std::tuple<AllocatedBuffer, std::vector<ShapeContainer>, BufferStatusType>> shapes;
private:
    std::shared_ptr<vulkan::Context> context;

    vk::DescriptorSetLayout descriptorSetLayout;

    void destroyShapeBuffer(const ShapeType &type);

    void recreateShapeBuffer(const ShapeType &type);

    void fillShapeBuffer(const ShapeType &type);

    [[nodiscard]] AllocatedBuffer allocateShapeBuffer(const ShapeType &type);

};


#endif //SPECTRALGRAPHER_SHAPEMANAGER_H
