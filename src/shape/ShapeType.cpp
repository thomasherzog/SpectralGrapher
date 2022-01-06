#include "shape/ShapeType.h"

ShapeType::ShapeType(std::string pluginId, std::string name, std::string sourceCode,
                     std::unordered_map<std::string, ShapeFieldType> fields)
        : pluginId(std::move(pluginId)),
          name(std::move(name)),
          sourceCode(std::move(sourceCode)),
          fields(std::move(fields)) {}

int ShapeType::getAlignedMemorySize() const {
    int size = 0;
    for (auto const &[fieldName, fieldType]: fields) {
        size += fieldType.getAlignedMemorySize();
    }
    return size;
}

size_t ShapeType::getUniqueId() const {
    return std::hash<ShapeType>()(*this);
}

std::string ShapeType::getUniqueName() const {
    return "SDF" + std::to_string(getUniqueId());
}

std::string ShapeType::generateGLSLStruct() const {
    std::string source = "struct " + getUniqueName() + " {\n";
    for (auto const &[fieldName, fieldType]: fields) {
        source += fieldType.getGLSLType();
        source += " " + fieldName + ";\n";
    }
    source += "}";
    return source;
}

std::string ShapeType::generateGLSLDistanceFunction() const {
    std::string source = "float getDistance" + getUniqueName() + "(vec3 point, ";
    source += getUniqueName() + " object) {\n";
    source += sourceCode;
    source += "\n}";
    return source;
}

std::string ShapeType::generateGLSLBufferDescription(int set, int binding) const {
    std::string source = "layout(std140, set = " + std::to_string(set) + ", ";
    source += "binding = " + std::to_string(binding) + ") ";
    source += "readonly buffer " + getUniqueName() + "BufferObject {\n";
    source += getUniqueName() + " objects[];\n";
    source += "} " + getUniqueName() + "Buffer;";
    return source;
}

std::string ShapeType::generateGLSLNormalFunction() const {
    auto distanceFunction = "getDistance" + getUniqueName();
    std::string source = "vec3 getNormal" + getUniqueName() + "(vec3 point, ";
    source += getUniqueName() + " object) {\n";
    source += "float distance = " + distanceFunction + "(point, object);\n";
    source += "return normalize(vec3(\n";
    source += distanceFunction + "(point + vec3(ubo.epsilon, 0, 0), object) - distance,\n";
    source += distanceFunction + "(point + vec3(0, ubo.epsilon, 0), object) - distance,\n";
    source += distanceFunction + "(point + vec3(0, 0, ubo.epsilon), object) - distance\n";
    source += "));\n";
    source += "}";
    return source;
}

bool ShapeType::operator==(const ShapeType &other) const {
    return (pluginId == other.pluginId && name == other.name);
}






