#include "shaders.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

Shader::Shader(GLenum type, const std::string& source)
    : Shader(type) {
    (*this).source(source).compile();
}

Shader::Shader(GLenum type)
    : _id(glCreateShader(type)) {
    if (_id == 0) {
        throw std::runtime_error("Failed to create shader");
    }
}

Shader::~Shader() noexcept {
    glDeleteShader(_id);
}

Shader::Shader(Shader&& other) noexcept
    : _id(other._id) {
    other._id = 0;
}
Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        glDeleteShader(_id);
        _id = other._id;
        other._id = 0;
    }
    return *this;
}

Shader& Shader::source(const std::string& source) noexcept {
    const char* src = source.c_str();
    glShaderSource(_id, 1, &src, nullptr);
    return *this;
}

Shader& Shader::compile() {
    glCompileShader(_id);

    GLint success;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(_id, 512, nullptr, infoLog);
        throw std::runtime_error(
            "Shader compilation failed: " + std::string(infoLog)
        );
    }

    return *this;
}

Program::Program()
    : _id(glCreateProgram()) {
    if (_id == 0) {
        throw std::runtime_error("Failed to create program");
    }
}

Program::~Program() noexcept {
    glDeleteProgram(_id);
}

Program::Program(Program&& other) noexcept
    : _id(other._id) {
    other._id = 0;
}

Program& Program::operator=(Program&& other) noexcept {
    if (this != &other) {
        glDeleteProgram(_id);
        _id = other._id;
        other._id = 0;
    }
    return *this;
}

Program& Program::attachShader(const Shader& shader) noexcept {
    glAttachShader(_id, shader.id());
    return *this;
}

Program& Program::link() {
    glLinkProgram(_id);

    GLint success;
    glGetProgramiv(_id, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(_id, 512, nullptr, infoLog);
        throw std::runtime_error(
            "Program linking failed: " + std::string(infoLog)
        );
    }

    return *this;
}

void Program::use() const noexcept {
    glUseProgram(_id);
}

UniformLocation Program::getUniformLocation(const std::string& name
) const noexcept {
    return UniformLocation(_id, name);
}

UniformLocation::UniformLocation(
    GLuint program, const std::string& name
) noexcept
    : _program(program),
      _location(glGetUniformLocation(program, name.c_str())) {}

void UniformLocation::set(GLfloat value) noexcept {
    glProgramUniform1f(_program, _location, value);
}

void UniformLocation::set(GLint value) noexcept {
    glProgramUniform1i(_program, _location, value);
}

void UniformLocation::set(GLuint value) noexcept {
    glProgramUniform1ui(_program, _location, value);
}

void UniformLocation::set(const glm::vec2& value) noexcept {
    glProgramUniform2fv(_program, _location, 1, glm::value_ptr(value));
}

void UniformLocation::set(const glm::vec3& value) noexcept {
    glProgramUniform3fv(_program, _location, 1, glm::value_ptr(value));
}

void UniformLocation::set(const glm::vec4& value) noexcept {
    glProgramUniform4fv(_program, _location, 1, glm::value_ptr(value));
}

void UniformLocation::set(const glm::mat2& value) noexcept {
    glProgramUniformMatrix2fv(
        _program, _location, 1, GL_FALSE, glm::value_ptr(value)
    );
}

void UniformLocation::set(const glm::mat3& value) noexcept {
    glProgramUniformMatrix3fv(
        _program, _location, 1, GL_FALSE, glm::value_ptr(value)
    );
}

void UniformLocation::set(const glm::mat4& value) noexcept {
    glProgramUniformMatrix4fv(
        _program, _location, 1, GL_FALSE, glm::value_ptr(value)
    );
}
