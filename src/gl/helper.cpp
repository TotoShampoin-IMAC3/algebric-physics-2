#include "helper.hpp"
#include <fstream>
#include <string>

Shader loadShader(GLenum type, const std::filesystem::path& path) {
    auto file = std::ifstream(path);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    auto source = std::string(
        std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()
    );

    return Shader(type, source);
}
