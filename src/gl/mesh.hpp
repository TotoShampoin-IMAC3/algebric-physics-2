#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};
struct Instance {
    glm::vec3 a;
    glm::vec3 b;
};

class Mesh {
public:
    Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<GLuint>& indices = {}
    );

    ~Mesh() noexcept;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    Mesh& bind() noexcept;
    Mesh& unbind() noexcept;
    Mesh& draw() noexcept;
    Mesh& drawInstanced(const std::vector<Instance>& instances) noexcept;

private:
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    GLuint _instanceVbo;
    GLsizei _count;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};
