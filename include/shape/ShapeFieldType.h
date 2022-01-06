#ifndef SPECTRALGRAPHER_SHAPEFIELDTYPE_H
#define SPECTRALGRAPHER_SHAPEFIELDTYPE_H

class ShapeFieldType {
public:
    enum Value {
        FLOAT,
        FLOAT3,
        SHAPE
    };

    ShapeFieldType() = default;

    ShapeFieldType(Value value);

    constexpr explicit operator Value() const;

    operator bool() = delete;

    constexpr bool operator==(ShapeFieldType type) const;

    constexpr bool operator!=(ShapeFieldType type) const;

    [[nodiscard]] const char *getGLSLType() const;

    [[nodiscard]] int getAlignedMemorySize() const;

private:
    Value value;

};

#endif //SPECTRALGRAPHER_SHAPEFIELDTYPE_H
