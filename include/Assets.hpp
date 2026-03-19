#pragma once

#include <glm/glm.hpp> 

#pragma pack(push, 1)
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    bool operator == (const Vertex& v1) const {
        return (position == v1.position
            && normal == v1.normal
            && texCoords == v1.texCoords);
    }
};
#pragma pack(pop)


