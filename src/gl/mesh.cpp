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
    glGenBuffers(1, &_instanceVbo);
    if (_instanceVbo == 0) {
        throw std::runtime_error("Failed to create instance buffer");
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

    glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);

    glVertexAttribPointer(
        3, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)offsetof(Instance, a)
    );
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glVertexAttribPointer(
        4, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)offsetof(Instance, b)
    );
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

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
      _instanceVbo(other._instanceVbo),
      _count(other._count) {
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
    other._instanceVbo = 0;
    other._count = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (_instanceVbo)
            glDeleteBuffers(1, &_instanceVbo);
        if (_ebo)
            glDeleteBuffers(1, &_ebo);
        glDeleteBuffers(1, &_vbo);
        glDeleteVertexArrays(1, &_vao);
        _vao = other._vao;
        _vbo = other._vbo;
        _ebo = other._ebo;
        _instanceVbo = other._instanceVbo;
        _count = other._count;
        other._vao = 0;
        other._vbo = 0;
        other._ebo = 0;
        other._instanceVbo = 0;
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

Mesh& Mesh::drawInstanced(const std::vector<Instance>& instances) noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, _instanceVbo);
    glBufferData(
        GL_ARRAY_BUFFER, instances.size() * sizeof(Instance), instances.data(),
        GL_STATIC_DRAW
    );
    if (_ebo != 0) {
        glDrawElementsInstanced(
            GL_TRIANGLES, _count, GL_UNSIGNED_INT, nullptr, instances.size()
        );
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, _count, instances.size());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return *this;
}
