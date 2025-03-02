#include "gl/helper.hpp"
#include "gl/mesh.hpp"
#include "gl/shaders.hpp"
#define GLFW_INCLUDE_NONE
#include <glfwpp/glfwpp.h>

#include <gl/gl.h>

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(800, 600, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Mesh triangle({
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        { {0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {  {0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
    });

    Program program;
    program.attachShader(loadShader(GL_VERTEX_SHADER, "res/basic.vert"))
        .attachShader(loadShader(GL_FRAGMENT_SHADER, "res/basic.frag"))
        .link();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.framebufferSizeEvent.setCallback([&](glfw::Window&, int width,
                                                int height) {
        glViewport(0, 0, width, height);
    });

    while (!window.shouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT);

        program.use();
        triangle.bind().draw().unbind();

        glfw::pollEvents();
        window.swapBuffers();
    }

    return 0;
}