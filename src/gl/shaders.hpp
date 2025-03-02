#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader() = delete;
    Shader(GLenum type);
    Shader(GLenum type, const std::string& source);
    ~Shader() noexcept;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    Shader& source(const std::string& source) noexcept;
    Shader& compile();

    GLuint id() const noexcept { return _id; }

private:
    GLuint _id;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    friend class Program;
};

class UniformLocation {
public:
    UniformLocation(GLuint program, const std::string& name) noexcept;

    void set(GLfloat value) noexcept;
    void set(GLint value) noexcept;
    void set(GLuint value) noexcept;
    void set(const glm::vec2& value) noexcept;
    void set(const glm::vec3& value) noexcept;
    void set(const glm::vec4& value) noexcept;
    void set(const glm::mat2& value) noexcept;
    void set(const glm::mat3& value) noexcept;
    void set(const glm::mat4& value) noexcept;

private:
    GLuint _program;
    GLint _location;
};

class Program {
public:
    Program();
    ~Program() noexcept;

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    Program& attachShader(const Shader& shader) noexcept;
    Program& link();

    void use() const noexcept;
    UniformLocation getUniformLocation(const std::string& name) const noexcept;

    GLuint id() const noexcept { return _id; }

private:
    GLuint _id;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};
