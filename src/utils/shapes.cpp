#include "shapes.hpp"
#include "../rendering/Displayator.hpp"
#include "glm/gtc/constants.hpp"
#include <algorithm>
#include <execution>
#include <glm/glm.hpp>
#include <ranges>
#include <utility>
#include <vector>

constexpr std::pair<glm::vec3, glm::vec3> cube_verts[] = {
    {   {1, 1, 1},   {1, 1, -1}},
    {   {1, 1, 1},   {1, -1, 1}},
    {   {1, 1, 1},   {-1, 1, 1}},
    {  {1, 1, -1},  {1, -1, -1}},
    {  {1, 1, -1},  {-1, 1, -1}},
    {  {-1, 1, 1},  {-1, 1, -1}},
    {  {-1, 1, 1},  {-1, -1, 1}},
    { {-1, 1, -1}, {-1, -1, -1}},
    { {-1, -1, 1},   {1, -1, 1}},
    { {-1, -1, 1}, {-1, -1, -1}},
    {{-1, -1, -1},  {1, -1, -1}},
    { {1, -1, -1},   {1, -1, 1}}
};
constexpr unsigned int sphere_segments = 64;

void drawAABB(
    Displayator& displayator, const glm::vec3& position, const glm::vec3& size,
    const glm::vec3& color
) {
    std::vector<std::pair<glm::vec3, glm::vec3>> lines(12);

    auto halfSize = size * 0.5f;
    std::for_each(
        std::execution::par_unseq, std::begin(cube_verts), std::end(cube_verts),
        [&](const auto& line) {
            lines.emplace_back(
                position + line.first * halfSize,
                position + line.second * halfSize
            );
        }
    );

    displayator.setColor(color).drawLines(lines);
}

void drawSphere(
    Displayator& displayator, const glm::vec3& position, float radius,
    const glm::vec3& color
) {
    std::vector<std::pair<glm::vec3, glm::vec3>> lines(sphere_segments * 3);
    auto range = std::views::iota(0U, sphere_segments);
    auto delta_angle = glm::tau<float>() / sphere_segments;

    std::for_each(
        std::execution::par_unseq, range.begin(), range.end(),
        [&](auto i) {
            auto a0 = i * delta_angle;
            auto a1 = (i + 1) * delta_angle;
            auto x0 = radius * glm::cos(a0);
            auto y0 = radius * glm::sin(a0);
            auto x1 = radius * glm::cos(a1);
            auto y1 = radius * glm::sin(a1);

            lines[i] = {
                glm::vec3 {x0, y0, 0}
                 + position,
                glm::vec3 {x1, y1, 0}
                 + position
            };
            lines[i + sphere_segments] = {
                glm::vec3 {x0, 0, y0}
                 + position,
                glm::vec3 {x1, 0, y1}
                 + position
            };
            lines[i + 2 * sphere_segments] = {
                glm::vec3 {0, x0, y0}
                 + position,
                glm::vec3 {0, x1, y1}
                 + position
            };
        }
    );

    displayator.setColor(color).drawLines(lines);
}