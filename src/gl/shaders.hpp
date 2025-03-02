#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <variant>

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
using UniformValue = std::variant<
    GLfloat, GLint, GLuint, glm::vec2, glm::vec3, glm::vec4, glm::mat2,
    glm::mat3, glm::mat4>;

class Program {
public:
    Program();
    ~Program() noexcept;

    Program(Program&& other) noexcept;
    Program& operator=(Program&& other) noexcept;

    Program& attachShader(const Shader& shader) noexcept;
    Program& link();

    Program& use() noexcept;
    UniformLocation getUniformLocation(const std::string& name) const noexcept;

    Program& setUniform(
        const std::string& name, const UniformValue& value
    ) noexcept;

    GLuint id() const noexcept { return _id; }

private:
    GLuint _id;

    std::unordered_map<std::string, UniformLocation> _uniforms;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};
