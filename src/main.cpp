// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include "glm/gtc/type_ptr.hpp"
#include "physics/density.hpp"
#include "utils/shapes.hpp"
#include <algorithm>
#include <execution>
#include <ranges>
#include <thread>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glfwpp/glfwpp.h>

#include "constants.hpp"

#include "physics/Time.hpp"
#include "physics/physics.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Displayator.hpp"
#include "utils/creators.hpp"
#include "utils/types.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <klein/klein.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ===== Profiling deduction =====
// The rendering as it is now is the main bottleneck.
// TODO: Support rendering of multiple objects in less calls.

// ===== Profiling deduction 2 =====
// Imgui tanks the performance, somehow.

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(WIDTH, HEIGHT, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    Displayator displayator;
    displayator.setProjection(glm::radians(45.0f), WIDTH / HEIGHT, NEAR, FAR);

    float windowWidth = WIDTH;
    float windowHeight = HEIGHT;

    bool windowRefresh = false;
    bool windowFocused = true;

    constexpr auto PI = glm::pi<float>();
    OrbitCamera camera(15.0f, PI / 8, PI / 4);
    camera.sensitivity = 5.f / WIDTH;

    float angle = 0.0f;
    bool isHolding = false;
    int nextCount = N;
    float gravityForce = GRAVITY;
    DrapeAnchors anchors = DrapeAnchors::Corners;

    glm::vec3 pinchDirection = {0, 1, 0};
    float pinchForce = PINCH_FORCE;
    int pinchIndex = 0;

    glm::vec4 groundFactors = {0, 1, 0, 5};

    unsigned int threads = std::thread::hardware_concurrency();

    std::vector<Particle> particles;
    std::vector<SpringLink> links;

    // Wall ground {kln::plane(0, 1, 0, 5), 100.f};
    Wall ground {
        kln::plane(
            groundFactors.x, groundFactors.y, groundFactors.z, groundFactors.w
        ),
        100.f
    };
    ConstantForce gravity {kln::translator(gravityForce, 0, -1, 0)};
    Spring spring {KNOT, STIFF, VISCOSITY};
    Density density {
        DENSITY, DENSITY_REPULSION, DENSITY_LOOKUP_RADIUS, DENSITY_GRID_SIZE
    };

    std::vector<glm::vec3> points;
    std::vector<std::pair<glm::vec3, glm::vec3>> lines;

    const auto reset = [&] {
        float mass = MASS;
        if (!particles.empty()) {
            mass = particles[0].mass;
        }
        particles.clear();
        links.clear();
        drape(particles, links, {nextCount, mass, KNOT, anchors});
        points.resize(particles.size());
        lines.resize(links.size());
        pinchIndex = nextCount * (nextCount + 1) / 2;
        density.setParticles(particles);
    };
    reset();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    window.refreshEvent.setCallback([&](glfw::Window&) { windowRefresh = true; }
    );
    window.focusEvent.setCallback([&](glfw::Window&, bool focused) {
        windowFocused = focused;
    });
    window.posEvent.setCallback([&](glfw::Window&, int x, int y) {
        windowRefresh = true;
    });
    window.framebufferSizeEvent.setCallback([&](glfw::Window&, int width,
                                                int height) {
        windowWidth = static_cast<float>(width);
        windowHeight = static_cast<float>(height);
        camera.sensitivity = 5.f / windowWidth;
        displayator.setProjection(
            glm::radians(45.0f), windowWidth / windowHeight, NEAR, FAR
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
    float lastProfilerTime = 0.0f;
    while (!window.shouldClose()) {
        float delta = time.deltaTime();
        angle += time.deltaTime();

        if (windowRefresh || !windowFocused) {
            windowRefresh = false;
            delta = 0;
        }

        profiler.begin();

        // Links preparation
        std::for_each(
            std::execution::par_unseq, links.begin(), links.end(),
            [&](auto& link) {
                spring.length = link.length;
                spring.prepareForce(particles[link.a], particles[link.b]);
            }
        );
        // Particles preparation
        std::for_each(
            std::execution::par_unseq, particles.begin(), particles.end(),
            [&](auto& particle) {
                gravity.prepareForce(particle);
                ground.prepareForce(particle);
                density.prepareForce(particle);
            }
        );
        // Particles update
        std::for_each(
            std::execution::par_unseq, particles.begin(), particles.end(),
            [&](auto& particle) {
                particle.updateForce(delta);
                particle.update(delta);
            }
        );

        profiler.tick(); // 0 = simulation

        std::transform(
            particles.begin(), particles.end(), points.begin(),
            [](const auto& particle) { return pointToVec(particle.position); }
        );
        std::transform(
            links.begin(), links.end(), lines.begin(),
            [&](const auto& link) {
                return std::pair(
                    pointToVec(particles[link.a].position),
                    pointToVec(particles[link.b].position)
                );
            }
        );

        profiler.tick(); // 1 = conversion

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        displayator.setView(camera.view())
            .setPointSize(POINT_SIZE)
            .setLineWidth(LINE_SIZE)
            .setPlaneSize(PLANE_SIZE);

        displayator.setColor({1, 1, 1}).drawPoints(points);
        displayator.setColor({0, 1, 0}).drawLines(lines);

        for (auto part :
             density.nearbyParticles(particles[pinchIndex].position)) {
            displayator.setColor({1, .5, 0})
                .setPointSize(POINT_SIZE * 1.5)
                .drawPoint(pointToVec(part->position));
        }

        displayator.setColor({1, 0, 0})
            .setPointSize(POINT_SIZE * 2)
            .drawPoint(particles[pinchIndex].position);

        displayator.setLineWidth(LINE_SIZE * 2);
        // for (auto cell : density.nearbyCells(particles[pinchIndex].position))
        // {
        //     drawAABB(
        //         displayator, density.cellInSpace(cell),
        //         {density.gridCellSize, density.gridCellSize,
        //          density.gridCellSize},
        //         {1, 1, 0}
        //     );
        // }
        // drawSphere(
        //     displayator, pointToVec(particles[pinchIndex].position),
        //     density.lookupRadius, {1, 0.75, 0}
        // );

        // DRAW THIS LAST, BECAUSE IT'S TRANSPARENT
        displayator.setColor({0, 0.5, 1}).drawPlane(ground.wall);

        profiler.tick(); // 2 = rendering

        float mass = particles[0].mass;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
        ImGui::Begin(
            "UI", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_AlwaysAutoResize
        );
        ImGui::SeparatorText("Profiling");
        ImGui::Text("Nb threads: %d", threads);
        ImGui::Text("FPS: %.2f", 1.0f / time.deltaTime());
        ImGui::Text("Time to simulate: %.5fs", profiler[0]);
        ImGui::Text("Time to convert : %.5fs", profiler[1]);
        ImGui::Text("Time to render  : %.5fs", profiler[2]);
        ImGui::Text("Time to imgui  : %.5fs", lastProfilerTime);
        ImGui::Text("N particles: %lld", particles.size());
        ImGui::Text("N links: %lld", links.size());
        ImGui::SeparatorText("Simulation");
        if (ImGui::Button("Reset")) {
            reset();
        }
        ImGui::SameLine();
        ImGui::InputInt("N", &nextCount);
        ImGui::InputFloat("Stiffness", &spring.stiffness);
        ImGui::InputFloat("Viscosity", &spring.viscosity);
        if (ImGui::InputFloat("Mass", &mass)) {
            std::for_each(
                std::execution::par_unseq, particles.begin(), particles.end(),
                [&](auto& particle) { particle.mass = mass; }
            );
        }
        if (ImGui::InputFloat("Gravity", &gravityForce)) {
            gravity.force = kln::translator(gravityForce, 0, -1, 0);
        }
        if (ImGui::BeginCombo("Anchors", to_string(anchors).c_str())) {
            for (const auto& anchor : drape_anchors) {
                if (ImGui::Selectable(
                        to_string(anchor).c_str(), anchor == anchors
                    )) {
                    anchors = anchor;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::End();

        ImGui::SetNextWindowPos({windowWidth, 0}, ImGuiCond_Always, {1, 0});
        ImGui::Begin(
            "UI 2", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_AlwaysAutoResize
        );
        ImGui::SeparatorText("Particle observer");
        if (ImGui::InputInt("Particle index", &pinchIndex)) {
            pinchIndex = (pinchIndex + particles.size()) % particles.size();
        }
        ImGui::Text(
            "Position: % 3.2f e013 + % 3.2f e021 + % 3.2f e032 + % 3.2f e123",
            particles[pinchIndex].position.e013(),
            particles[pinchIndex].position.e021(),
            particles[pinchIndex].position.e032(),
            particles[pinchIndex].position.e123()
        );
        ImGui::Text("Velocity:");
        ImGui::Text(
            "% 3.2f + % 3.2f e01 + % 3.2f e02 + % 3.2f e03",
            particles[pinchIndex].velocity.scalar(),
            particles[pinchIndex].velocity.e01(),
            particles[pinchIndex].velocity.e02(),
            particles[pinchIndex].velocity.e03()
        );
        ImGui::Text(
            "      + % 3.2f e10 + % 3.2f e20 + % 3.2f e30",
            particles[pinchIndex].velocity.e10(),
            particles[pinchIndex].velocity.e20(),
            particles[pinchIndex].velocity.e30()
        );
        glm::vec3 pinchDirectionNormalized;
        ImGui::SliderFloat3(
            "Pinch direction (will be normalized)",
            glm::value_ptr(pinchDirection), -1.f, 1.f
        );
        pinchDirectionNormalized = glm::normalize(pinchDirection);
        auto pinchForceVec = kln::translator(
            pinchForce, pinchDirectionNormalized.x, pinchDirectionNormalized.y,
            pinchDirectionNormalized.z
        );
        ImGui::InputFloat("Pinch force", &pinchForce);
        if (ImGui::Button("Pinch")) {
            particles[pinchIndex].applyForce(pinchForceVec, time.deltaTime());
        }
        ImGui::Text("Pinch force: ");
        ImGui::Text(
            "% 10.2f + % 10.2f e01 + % 10.2f e02 + % 10.2f e03",
            pinchForceVec.scalar(), pinchForceVec.e01(), pinchForceVec.e02(),
            pinchForceVec.e03()
        );
        ImGui::Text(
            "           + % 10.2f e10 + % 10.2f e20 + % 10.2f e30",
            pinchForceVec.e10(), pinchForceVec.e20(), pinchForceVec.e30()
        );
        ImGui::SeparatorText("Ground");
        // if (ImGui::SliderFloat4(
        //         "Ground factors", glm::value_ptr(groundFactors), -1.f, 1.f
        //     )) {
        if (ImGui::SliderFloat("e0", &groundFactors.x, -1.f, 1.f) ||
            ImGui::SliderFloat("e1", &groundFactors.y, -1.f, 1.f) ||
            ImGui::SliderFloat("e2", &groundFactors.z, -1.f, 1.f) ||
            ImGui::SliderFloat("e3", &groundFactors.w, -5.f, 5.f)) {
            ground.wall = kln::plane(
                groundFactors.x, groundFactors.y, groundFactors.z,
                groundFactors.w
            );
        }
        if (ImGui::InputFloat("Ground force", &ground.force)) {
            ground.force = ground.force;
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        profiler.tick(); // 3 = imgui
        lastProfilerTime = profiler[3];

        glfw::pollEvents();
        window.swapBuffers();
        time.tick();
    }

    return 0;
}
