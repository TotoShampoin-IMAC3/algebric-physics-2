#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
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

private:
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    GLsizei _count;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};
