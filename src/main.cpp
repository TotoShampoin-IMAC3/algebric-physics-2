// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glfwpp/glfwpp.h>

#include "physics/physics.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Displayator.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <klein/klein.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

const float WIDTH = 800.f;
const float HEIGHT = 600.f;

const size_t N = 50;
const float KNOT = .1f;
const float STIFF = 1000.f;
const float MASS = 0.1f;

const float SIZE = 0.05f;

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(WIDTH, HEIGHT, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Displayator displayator;
    displayator.setProjection(
        glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f
    );

    OrbitCamera camera;
    camera.sensitivity = 5.f / WIDTH;

    float angle = 0.0f;
    bool isHolding = false;

    std::vector<Particle> particles;
    Wall ground {kln::plane(0, 1, 0, 5), 100.f};
    ConstantForce gravity {kln::translator(10.f, 0, -1, 0)};
    Viscosity viscosity {0.25f};
    Spring spring {KNOT, STIFF};

    for (int i = 0; i < N; i++) {
        particles.emplace_back(kln::point((i - int(N / 2)) * KNOT, 0, 0), MASS);
    }
    particles[0].lock = true;
    particles[N - 1].lock = true;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.framebufferSizeEvent.setCallback([&](glfw::Window&, int width,
                                                int height) {
        float widthf = static_cast<float>(width);
        float heightf = static_cast<float>(height);
        camera.sensitivity = 5.f / widthf;
        displayator.setProjection(
            glm::radians(45.0f), widthf / heightf, 0.1f, 100.0f
        );
        glViewport(0, 0, width, height);
    });
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Time time;
    while (!window.shouldClose()) {
        angle += time.deltaTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        displayator.setView(camera.view());

        displayator.setColor({1, 1, 1});
        displayator.setPlaneSize(10.0f).drawPlane(ground.wall);
        displayator.setPointSize(SIZE).setLineWidth(SIZE);
        for (int i = 0; i < N - 1; i++) {
            spring.applyForce(time.deltaTime(), particles[i], particles[i + 1]);
            displayator.drawLine(
                particles[i].position, particles[i + 1].position
            );
        }
        for (auto& particle : particles) {
            gravity.applyForce(time.deltaTime(), particle);
            ground.applyForce(time.deltaTime(), particle);
            viscosity.applyForce(time.deltaTime(), particle);

            particle.update(time.deltaTime());
            displayator.drawPoint(particle.position);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfw::pollEvents();
        window.swapBuffers();
        time.tick();
    }

    return 0;
}
