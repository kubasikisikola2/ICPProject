#include <memory>
#include "Mesh.hpp"
#include "MeshGen.hpp"

std::shared_ptr<Mesh> generate_cube_mesh() {
    std::vector<Vertex> V{
        // Front face (+Z)
        {{-0.5f, -0.5f,  0.5f}, {0,0,1}, {0,0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,0,1}, {1,0}},
        {{ 0.5f,  0.5f,  0.5f}, {0,0,1}, {1,1}},
        {{-0.5f,  0.5f,  0.5f}, {0,0,1}, {0,1}},

        // Back face (-Z)
        {{ 0.5f, -0.5f, -0.5f}, {0,0,-1}, {0,0}},
        {{-0.5f, -0.5f, -0.5f}, {0,0,-1}, {1,0}},
        {{-0.5f,  0.5f, -0.5f}, {0,0,-1}, {1,1}},
        {{ 0.5f,  0.5f, -0.5f}, {0,0,-1}, {0,1}},

        // Left face (-X)
        {{-0.5f, -0.5f, -0.5f}, {-1,0,0}, {0,0}},
        {{-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1,0}},
        {{-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1,1}},
        {{-0.5f,  0.5f, -0.5f}, {-1,0,0}, {0,1}},

        // Right face (+X)
        {{ 0.5f, -0.5f,  0.5f}, {1,0,0}, {0,0}},
        {{ 0.5f, -0.5f, -0.5f}, {1,0,0}, {1,0}},
        {{ 0.5f,  0.5f, -0.5f}, {1,0,0}, {1,1}},
        {{ 0.5f,  0.5f,  0.5f}, {1,0,0}, {0,1}},

        // Top face (+Y)
        {{-0.5f,  0.5f,  0.5f}, {0,1,0}, {0,0}},
        {{ 0.5f,  0.5f,  0.5f}, {0,1,0}, {1,0}},
        {{ 0.5f,  0.5f, -0.5f}, {0,1,0}, {1,1}},
        {{-0.5f,  0.5f, -0.5f}, {0,1,0}, {0,1}},

        // Bottom face (-Y)
        {{-0.5f, -0.5f, -0.5f}, {0,-1,0}, {0,0}},
        {{ 0.5f, -0.5f, -0.5f}, {0,-1,0}, {1,0}},
        {{ 0.5f, -0.5f,  0.5f}, {0,-1,0}, {1,1}},
        {{-0.5f, -0.5f,  0.5f}, {0,-1,0}, {0,1}},
    };

    std::vector<GLuint> I{
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9,10,10,11, 8,       // Left
       12,13,14,14,15,12,       // Right
       16,17,18,18,19,16,       // Top
       20,21,22,22,23,20        // Bottom
    };

    return std::make_shared<Mesh>(V, I, GL_TRIANGLES);
}


std::shared_ptr<Mesh> generate_quad_mesh(
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& p2,
    const glm::vec3& p3,
    const glm::vec3& normal,
    float uvScaleX,
    float uvScaleY)
{
    std::vector<Vertex> vertices = {
        {p0, normal, {0.0f, 0.0f}},
        {p1, normal, {uvScaleX, 0.0f}},
        {p2, normal, {uvScaleX, uvScaleY}},
        {p3, normal, {0.0f, uvScaleY}}
    };

    std::vector<GLuint> indices = {
        0, 1, 2,
        2, 3, 0
    };

    return std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
}

std::shared_ptr<Mesh> generate_webcam_mesh()
{
    std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {0,0,1}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {0,0,1}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {0,0,1}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {0,0,1}, {0.0f, 1.0f}}
    };

    std::vector<GLuint> indices = {
        0, 1, 2,
        2, 3, 0
    };

    return std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
}