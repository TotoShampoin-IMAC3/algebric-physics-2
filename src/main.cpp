// DO NOT REMOVE
// This file is there because glfwpp needs it for std::exchange
#include <exception>
#include <execution>

#include "glm/gtc/type_ptr.hpp"
#include "physics/density.hpp"
#include "utils/shapes.hpp"
#include <algorithm>
#include <mutex>
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

// ===== Profiling deduction 3 =====
// By far, the hashmap lookup is the biggest bottleneck of the simulation.

int main(int argc, const char* argv[]) {
    auto GLFW = glfw::init();
    glfw::Window window(WIDTH, HEIGHT, "Hello World");

    glfw::makeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfw::getProcAddress);

    glfw::swapInterval(1);

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

    struct PhysicsData {
        std::vector<Particle> particles;
        std::vector<SpringLink> links;
        Wall ground;
        ConstantForce gravity;
        Spring spring {KNOT, STIFF, VISCOSITY};
        Density density {
            DENSITY_REPULSION, DENSITY_LOOKUP_RADIUS, DENSITY_GRID_SIZE
        };

        std::mutex mutex;
    } pd {
        .ground =
            {kln::plane(
                 groundFactors.x, groundFactors.y, groundFactors.z,
                     groundFactors.w
             ), 100.f},
        .gravity = {kln::translator(gravityForce, 0, -1, 0)}
    };
    // auto& particles = pd.particles;
    // auto& links = pd.links;
    // auto& ground = pd.ground;
    // auto& gravity = pd.gravity;
    // auto& spring = pd.spring;
    // auto& density = pd.density;

    struct RenderData {
        std::vector<glm::vec3> points;
        std::vector<std::pair<glm::vec3, glm::vec3>> lines;
        Density density;

        std::mutex mutex;
    } rd;

    auto& points = rd.points;
    auto& lines = rd.lines;

    float mass = MASS;
    const auto reset = [&] {
        pd.particles.clear();
        pd.links.clear();
        drape(pd.particles, pd.links, {nextCount, mass, KNOT, anchors});
        points.resize(pd.particles.size());
        lines.resize(pd.links.size());
        pinchIndex = nextCount * (nextCount + 1) / 2;
        pd.density.setParticles(pd.particles);
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

    bool terminate = false;
    bool callReset = false;
    float physicsDeltatime;
    std::vector<float> profilingData(5);
    auto physicsThread = std::thread([&] {
        auto& particles = pd.particles;
        auto& links = pd.links;
        auto& ground = pd.ground;
        auto& gravity = pd.gravity;
        auto& spring = pd.spring;
        auto& density = pd.density;

        Profiler profiler;
        Time time;
        while (!terminate) {
            float delta = time.deltaTime();
            physicsDeltatime = delta;

            pd.mutex.lock();

            if (callReset) {
                reset();
                callReset = false;
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
            profiler.tick();
            // Particles preparation
            std::for_each(
                std::execution::par_unseq, particles.begin(), particles.end(),
                [&](auto& particle) {
                    gravity.prepareForce(particle);
                    ground.prepareForce(particle);
                    density.prepareForce(particle);
                }
            );
            profiler.tick();
            // Particles update
            std::for_each(
                std::execution::par_unseq, particles.begin(), particles.end(),
                [&](auto& particle) {
                    particle.updateForce(delta);
                    particle.update(delta);
                }
            );
            profiler.tick();

            pd.mutex.unlock();
            rd.mutex.lock();

            std::transform(
                std::execution::par_unseq, particles.begin(), particles.end(),
                points.begin(),
                [](const auto& particle) {
                    return pointToVec(particle.position);
                }
            );
            profiler.tick();
            std::transform(
                std::execution::par_unseq, links.begin(), links.end(),
                lines.begin(),
                [&](const auto& link) {
                    return std::pair(
                        pointToVec(particles[link.a].position),
                        pointToVec(particles[link.b].position)
                    );
                }
            );
            profiler.tick();
            rd.density = pd.density;
            profiler.tick();

            rd.mutex.unlock();

            for (int i = 0; i < 5; i++) {
                profilingData[i] = profiler[i];
            }

            time.tick();
        }
    });

    Time time;
    while (!window.shouldClose()) {
        float delta = time.deltaTime();
        angle += time.deltaTime();

        if (windowRefresh || !windowFocused) {
            windowRefresh = false;
            delta = 0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        displayator.setView(camera.view())
            .setPointSize(POINT_SIZE)
            .setLineWidth(LINE_SIZE)
            .setPlaneSize(PLANE_SIZE);

        displayator.setColor({1, 1, 1}).drawPoints(points);
        displayator.setColor({0, 1, 0}).drawLines(lines);

        for (auto part :
             rd.density.nearbyParticles(pd.particles[pinchIndex].position)) {
            displayator.setColor({1, .5, 0})
                .setPointSize(POINT_SIZE * 1.5)
                .drawPoint(pointToVec(part->position));
        }

        displayator.setColor({1, 0, 0})
            .setPointSize(POINT_SIZE * 2)
            .drawPoint(pd.particles[pinchIndex].position);

        // displayator.setLineWidth(LINE_SIZE * 2);
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
        displayator.setColor({0, 0.5, 1}).drawPlane(pd.ground.wall);

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
        ImGui::Text("Physics FPS: %.2f", 1.0f / physicsDeltatime);
        ImGui::Text("Links prep      : %.4fms", profilingData[0] * 1000.f);
        ImGui::Text("Particles prep  : %.4fms", profilingData[1] * 1000.f);
        ImGui::Text("Particles update: %.4fms", profilingData[2] * 1000.f);
        ImGui::Text("Points transform: %.4fms", profilingData[3] * 1000.f);
        ImGui::Text("Lines transform : %.4fms", profilingData[4] * 1000.f);
        ImGui::Text("Density copy    : %.4fms", profilingData[5] * 1000.f);
        ImGui::Text("N particles: %lld", pd.particles.size());
        ImGui::Text("N links: %lld", pd.links.size());
        ImGui::SeparatorText("Simulation");
        if (ImGui::Button("Reset")) {
            callReset = true;
        }
        ImGui::SameLine();
        ImGui::InputInt("N", &nextCount);
        ImGui::InputFloat("Stiffness", &pd.spring.stiffness);
        ImGui::InputFloat("Viscosity", &pd.spring.viscosity);
        if (ImGui::InputFloat("Mass", &mass)) {
            std::for_each(
                std::execution::par_unseq, pd.particles.begin(),
                pd.particles.end(),
                [&](auto& particle) { particle.mass = mass; }
            );
        }
        if (ImGui::InputFloat("Gravity", &gravityForce)) {
            pd.gravity.force = kln::translator(gravityForce, 0, -1, 0);
        }
        ImGui::InputFloat("Avoid force", &pd.density.repulsionFactor);
        ImGui::InputFloat("Avoid radius", &pd.density.lookupRadius);
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
            pinchIndex =
                (pinchIndex + pd.particles.size()) % pd.particles.size();
        }
        ImGui::Text(
            "Position: % 3.2f e013 + % 3.2f e021 + % 3.2f e032 + % 3.2f e123",
            pd.particles[pinchIndex].position.e013(),
            pd.particles[pinchIndex].position.e021(),
            pd.particles[pinchIndex].position.e032(),
            pd.particles[pinchIndex].position.e123()
        );
        ImGui::Text("Velocity:");
        ImGui::Text(
            "% 3.2f + % 3.2f e01 + % 3.2f e02 + % 3.2f e03",
            pd.particles[pinchIndex].velocity.scalar(),
            pd.particles[pinchIndex].velocity.e01(),
            pd.particles[pinchIndex].velocity.e02(),
            pd.particles[pinchIndex].velocity.e03()
        );
        ImGui::Text(
            "      + % 3.2f e10 + % 3.2f e20 + % 3.2f e30",
            pd.particles[pinchIndex].velocity.e10(),
            pd.particles[pinchIndex].velocity.e20(),
            pd.particles[pinchIndex].velocity.e30()
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
            pd.particles[pinchIndex].applyForce(
                pinchForceVec, time.deltaTime()
            );
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
            pd.ground.wall = kln::plane(
                groundFactors.x, groundFactors.y, groundFactors.z,
                groundFactors.w
            );
        }
        if (ImGui::InputFloat("Ground force", &pd.ground.force)) {
            pd.ground.force = pd.ground.force;
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
