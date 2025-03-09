// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include <utility>

#define GLFW_INCLUDE_NONE
#include <glfwpp/glfwpp.h>

#include "rendering/Camera.hpp"
#include "rendering/Displayator.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <klein/klein.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(800, 600, "Hello World");
    float width = 800, height = 600;

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Displayator displayator;

    OrbitCamera camera;
    camera.sensitivity = 5.f / width;

    float angle = 0.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.framebufferSizeEvent.setCallback( //
        [&](glfw::Window&, int _width, int _height) {
            width = static_cast<float>(_width);
            height = static_cast<float>(_height);
            camera.sensitivity = 5.f / _width;
            glViewport(0, 0, _width, _height);
        }
    );

    bool isHolding = false;
    window.mouseButtonEvent.setCallback(
        [&](glfw::Window&, glfw::MouseButton button,
            glfw::MouseButtonState action, glfw::ModifierKeyBit) {
            if (button == glfw::MouseButton::Left) {
                isHolding = action == glfw::MouseButtonState::Press;
            }
        }
    );
    window.cursorPosEvent.setCallback([&](glfw::Window&, double x, double y) {
        static double lastX = x;
        static double lastY = y;
        if (isHolding) {
            double dx = x - lastX;
            double dy = y - lastY;
            camera.rotate(dy, dx);
        }
        lastX = x;
        lastY = y;
    });
    window.scrollEvent.setCallback([&](glfw::Window&, double, double y) {
        camera.zoom(-y);
    });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    displayator.setPointSize(0.20f).setLineWidth(0.02f).setPlaneSize(10.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double start = glfw::getTime();
    double last = start;
    while (!window.shouldClose()) {
        double now = glfw::getTime();
        double delta = now - last;
        double elapsed = now - start;
        last = now;

        angle += delta;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        displayator
            .setProjection(glm::radians(45.0f), width / height, 0.1f, 100.0f)
            .setView(camera.view());
        displayator.setColor(glm::vec3(1, 0, 0))
            .drawLine(kln::point(0, 0, 0) & kln::point(1, 0, 0), 1)
            .drawPoint(kln::point(1, 0, 0))
            .drawPoint(kln::point(-1, 0, 0));
        displayator.setColor(glm::vec3(0, 1, 0))
            .drawLine(kln::point(0, 0, 0) & kln::point(0, 1, 0), 1)
            .drawPoint(kln::point(0, 1, 0))
            .drawPoint(kln::point(0, -1, 0));
        displayator.setColor(glm::vec3(0, 0, 1))
            .drawLine(kln::point(0, 0, 0) & kln::point(0, 0, 1), 1)
            .drawPoint(kln::point(0, 0, 1))
            .drawPoint(kln::point(0, 0, -1));
        displayator.setColor(glm::vec3(1, 1, 1))
            .drawPlane(kln::plane(.1, 1, 0, 0));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfw::pollEvents();
        window.swapBuffers();
    }

    return 0;
}
