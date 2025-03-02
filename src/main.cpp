// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include <utility>

#define GLFW_INCLUDE_NONE
#include <glfwpp/glfwpp.h>

#include "rendering/Camera.hpp"
#include "rendering/Displayator.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(800, 600, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Displayator displayator;

    OrbitCamera camera;

    float angle = 0.0f;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.framebufferSizeEvent.setCallback([&](glfw::Window&, int width,
                                                int height) {
        glViewport(0, 0, width, height);
    });

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

    displayator
        .setProjection(
            glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f)
        )
        .setPointSize(0.20f)
        .setLineWidth(0.02f)
        .setPlaneSize(10.0f);

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

        // glm::mat4 view = glm::lookAt(
        //     glm::vec3(glm::cos(angle) * 5.0f, 2.0f, glm::sin(angle) * 5.0f),
        //     glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
        // );

        // displayator.setView(view);
        displayator.setView(camera.view());
        displayator.setColor({1.0f, 0.0f, 0.0f})
            .drawLine({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f})
            .drawPoint({1.0f, 0.0f, 0.0f})
            .drawPoint({-1.0f, 0.0f, 0.0f});
        displayator.setColor({0.0f, 1.0f, 0.0f})
            .drawLine({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f})
            .drawPoint({0.0f, 1.0f, 0.0f})
            .drawPoint({0.0f, -1.0f, 0.0f});
        displayator.setColor({0.0f, 0.0f, 1.0f})
            .drawLine({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f})
            .drawPoint({0.0f, 0.0f, 1.0f})
            .drawPoint({0.0f, 0.0f, -1.0f});

        displayator.setColor({1.0f, 1.0f, 1.0f})
            .drawPlane({0.0f, 0.0f, 0.0f}, {0.1f, 1.0f, 0.0f});

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
