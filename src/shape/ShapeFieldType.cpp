#include "shape/ShapeFieldType.h"

ShapeFieldType::ShapeFieldType(ShapeFieldType::Value value) : value(value) {}

constexpr ShapeFieldType::operator Value() const {
    return value;
}

constexpr bool ShapeFieldType::operator==(ShapeFieldType type) const {
    return value == type.value;
}

constexpr bool ShapeFieldType::operator!=(ShapeFieldType type) const {
    return value != type.value;
}

const char *ShapeFieldType::getGLSLType() const {
    switch (value) {
        case FLOAT:
            return "float";
        case FLOAT3:
            return "vec3";
        case SHAPE:
            return "SignedDistanceField";
    }
}

int ShapeFieldType::getAlignedMemorySize() const {
    switch (value) {
        case FLOAT:
            return 4;
        case FLOAT3:
            return 16;
        case SHAPE:
            return 8;
    }
}
