#pragma once

#include "shaders.hpp"
#include <filesystem>

Shader loadShader(GLenum type, const std::filesystem::path& path);
