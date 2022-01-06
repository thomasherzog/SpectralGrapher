#ifndef SPECTRALGRAPHER_SHAPETYPE_H
#define SPECTRALGRAPHER_SHAPETYPE_H

#include <string>
#include <unordered_map>

#include "shape/ShapeFieldType.h"

class ShapeType {
public:
    ShapeType(std::string pluginId, std::string name, std::string sourceCode,
              std::unordered_map<std::string, ShapeFieldType> fields);

    std::string pluginId;
    std::string name;
    std::string sourceCode;

    std::unordered_map<std::string, ShapeFieldType> fields;

    [[nodiscard]] int getAlignedMemorySize() const ;

    [[nodiscard]] size_t getUniqueId() const;

    [[nodiscard]] std::string getUniqueName() const;

    [[nodiscard]] std::string generateGLSLStruct() const;

    [[nodiscard]] std::string generateGLSLDistanceFunction() const;

    [[nodiscard]] std::string generateGLSLNormalFunction() const;

    [[nodiscard]] std::string generateGLSLBufferDescription(int set, int binding) const;

    bool operator==(const ShapeType &other) const;

};

namespace std {
    template<>
    struct hash<ShapeType> {
        size_t operator()(const ShapeType &type) const {
            size_t res = 17;
            res = res * 31 + hash<string>()(type.pluginId);
            res = res * 31 + hash<string>()(type.name);
            return res;
        }
    };
}


#endif //SPECTRALGRAPHER_SHAPETYPE_H
