// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include "physics/Time.hpp"
#include <algorithm>
#include <execution>
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

#include <ranges>

const float WIDTH = 800.f;
const float HEIGHT = 600.f;

const int N = 5;
const float KNOT = .5f;
const float STIFF = 2000.f;
const float MASS = .1f;
const float VISCOSITY = 1.f;
const float GRAVITY = 0.0f;

const float PINCH_FORCE = 100000.0f;

const float POINT_SIZE = 0.05f;
const float LINE_SIZE = 0.025f;

// ===== Profiling deduction =====
// The rendering as it is now is the main bottleneck.
// TODO: Support rendering of multiple objects in less calls.

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(WIDTH, HEIGHT, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Displayator displayator;
    displayator.setProjection(
        glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f
    );

    bool windowRefresh = false;

    constexpr auto PI = glm::pi<float>();
    OrbitCamera camera(15.0f, PI / 8, PI / 4);
    camera.sensitivity = 5.f / WIDTH;

    float angle = 0.0f;
    bool isHolding = false;
    int nextCount = N;
    float pinchForce = PINCH_FORCE;
    float gravityForce = GRAVITY;
    int pinchIndex = 0;

    struct SpringLink {
        int a;
        int b;
        float length;
    };

    std::vector<Particle> particles;
    std::vector<SpringLink> links;

    Wall ground {kln::plane(0, 1, 0, 5), 100.f};
    ConstantForce gravity {kln::translator(gravityForce, 0, -1, 0)};
    Spring spring {KNOT, STIFF, VISCOSITY};

    const auto reset = [&] {
        float mass = MASS;
        if (!particles.empty()) {
            mass = particles[0].mass;
        }
        particles.clear();
        links.clear();
        // for (int i = 0; i < N; i++) {
        //     particles.emplace_back(
        //         kln::point((i - int(N / 2)) * KNOT, 0, 0), mass
        //     );
        //     if (i > 0) {
        //         links.emplace_back(i - 1, i);
        //     }
        // }
        for (const auto& i : std::views::iota(0, nextCount)) {
            for (const auto& j : std::views::iota(0, nextCount)) {
                particles.emplace_back(
                    kln::point(
                        (i - int(nextCount / 2)) * KNOT, 0,
                        (j - int(nextCount / 2)) * KNOT
                    ),
                    mass
                );
                if (i > 0) {
                    links.emplace_back(
                        (i - 1) * nextCount + j, i * nextCount + j, KNOT
                    );
                }
                if (j > 0) {
                    links.emplace_back(
                        i * nextCount + (j - 1), i * nextCount + j, KNOT
                    );
                }
                if (i > 0 && j > 0) {
                    links.emplace_back(
                        (i - 1) * nextCount + (j - 1), i * nextCount + j,
                        KNOT * M_SQRT2
                    );
                    links.emplace_back(
                        i * nextCount + (j - 1), (i - 1) * nextCount + j,
                        KNOT * M_SQRT2
                    );
                }
                // if ((i == 0 && j == 0) || (i == 0 && j == nextCount - 1) ||
                //     (i == nextCount - 1 && j == 0) ||
                //     (i == nextCount - 1 && j == nextCount - 1)) {
                //     particles.back().lock = true;
                // }
                if ((i == 0 && j == 0) || (i == nextCount - 1 && j == 0)) {
                    particles.back().lock = true;
                }
            }
        }
    };
    reset();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.refreshEvent.setCallback([&](glfw::Window&) { windowRefresh = true; }
    );
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
            if (button == glfw::MouseButton::Left &&
                !ImGui::GetIO().WantCaptureMouse) {
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
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Time time;
    Profiler profiler;
    while (!window.shouldClose()) {
        float delta = time.deltaTime();
        angle += time.deltaTime();

        if (windowRefresh) {
            windowRefresh = false;
            delta = 0;
        }

        // for (auto& link : links) {
        //     spring.length = link.length;
        //     spring.prepareForce(particles[link.a], particles[link.b]);
        // }
        // for (auto& particle : particles) {
        //     gravity.prepareForce(particle);
        //     ground.prepareForce(particle);
        // }
        // for (auto& particle : particles) {
        //     particle.updateForce(delta);
        //     particle.update(delta);
        // }

        profiler.begin();

        std::for_each(
            std::execution::par_unseq, links.begin(), links.end(),
            [&](auto& link) {
                spring.length = link.length;
                spring.prepareForce(particles[link.a], particles[link.b]);
            }
        );
        std::for_each(
            std::execution::par_unseq, particles.begin(), particles.end(),
            [&](auto& particle) {
                gravity.prepareForce(particle);
                ground.prepareForce(particle);
            }
        );
        std::for_each(
            std::execution::par_unseq, particles.begin(), particles.end(),
            [&](auto& particle) {
                particle.updateForce(delta);
                particle.update(delta);
            }
        );

        profiler.tick(); // 0 = simulation

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        displayator.setView(camera.view());

        displayator.setColor({1, 1, 1});
        displayator.setPlaneSize(10.0f).drawPlane(ground.wall);
        displayator.setPointSize(POINT_SIZE).setLineWidth(LINE_SIZE);
        for (auto& particle : particles) {
            if (particle.lock) {
                displayator.setColor({1, 1, 0});
            } else {
                displayator.setColor({1, 1, 1});
            }
            displayator.drawPoint(particle.position);
        }
        for (auto& link : links) {
            auto length = link.length;
            auto dist =
                (particles[link.a].position & particles[link.b].position)
                    .norm();
            float fac = glm::max<float>(0, 1 - (dist - length));
            glm::vec3 color =
                glm::mix(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), fac);
            displayator //
                .setColor(color)
                .drawLine(
                    particles[link.a].position, particles[link.b].position
                );
        }

        profiler.tick(); // 1 = rendering

        float mass = particles[0].mass;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("UI");
        ImGui::SeparatorText("Profiling");
        ImGui::Text("FPS: %.2f", 1.0f / time.deltaTime());
        ImGui::Text("Time to simulate: %.5fs", profiler[0]);
        ImGui::Text("Time to render: %.5fs", profiler[1]);
        ImGui::Text("N : %d", N);
        ImGui::Text("N particles: %lld", particles.size());
        ImGui::Text("N links: %lld", links.size());
        ImGui::SeparatorText("Simulation");
        if (ImGui::Button("Reset")) {
            reset();
        }
        ImGui::SameLine();
        ImGui::InputInt("N", &nextCount);
        if (ImGui::Button("Pinch")) {
            particles[pinchIndex].applyForce(
                kln::translator(pinchForce, 0, 0, 1), time.deltaTime()
            );
        }
        ImGui::InputFloat("Pinch force", &pinchForce);
        if (ImGui::InputInt("Pinch index", &pinchIndex)) {
            pinchIndex = (pinchIndex + N) % N;
        }
        ImGui::Separator();
        ImGui::InputFloat("Stiffness", &spring.stiffness);
        ImGui::InputFloat("Viscosity", &spring.viscosity);
        if (ImGui::InputFloat("Mass", &mass)) {
            for (auto& particle : particles) {
                particle.mass = mass;
            }
        }
        if (ImGui::InputFloat("Gravity", &gravityForce)) {
            gravity.force = kln::translator(gravityForce, 0, -1, 0);
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfw::pollEvents();
        window.swapBuffers();
        time.tick();
    }

    return 0;
}
