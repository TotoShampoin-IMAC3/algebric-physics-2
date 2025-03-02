#include "mesh.hpp"
#include <stdexcept>

Mesh::Mesh(
    const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices
) {
    glGenVertexArrays(1, &_vao);
    if (_vao == 0) {
        throw std::runtime_error("Failed to create vertex array");
    }
    glGenBuffers(1, &_vbo);
    if (_vbo == 0) {
        throw std::runtime_error("Failed to create vertex buffer");
    }
    if (indices.size() > 0) {
        glGenBuffers(1, &_ebo);
        if (_ebo == 0) {
            throw std::runtime_error("Failed to create element buffer");
        }
    }

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
        GL_STATIC_DRAW
    );

    if (indices.size() > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
            indices.data(), GL_STATIC_DRAW
        );
    }

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, position)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal)
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord)
    );
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    if (indices.size() > 0) {
        _count = indices.size();
    } else {
        _count = vertices.size();
    }
}

Mesh::~Mesh() noexcept {
    if (_ebo != 0) {
        glDeleteBuffers(1, &_ebo);
    }
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

Mesh::Mesh(Mesh&& other) noexcept
    : _vao(other._vao),
      _vbo(other._vbo),
      _ebo(other._ebo),
      _count(other._count) {
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
    other._count = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        glDeleteBuffers(1, &_ebo);
        glDeleteBuffers(1, &_vbo);
        glDeleteVertexArrays(1, &_vao);
        _vao = other._vao;
        _vbo = other._vbo;
        _ebo = other._ebo;
        _count = other._count;
        other._vao = 0;
        other._vbo = 0;
        other._ebo = 0;
        other._count = 0;
    }
    return *this;
}

Mesh& Mesh::bind() noexcept {
    glBindVertexArray(_vao);
    return *this;
}

Mesh& Mesh::unbind() noexcept {
    glBindVertexArray(0);
    return *this;
}

Mesh& Mesh::draw() noexcept {
    if (_ebo != 0) {
        glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, _count);
    }
    return *this;
}
