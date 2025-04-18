#pragma once

#include "rendering/Displayator.hpp"
#include <glm/glm.hpp>

void drawAABB(
    Displayator&, const glm::vec3& position, const glm::vec3& size,
    const glm::vec3& color
);

void drawSphere(
    Displayator&, const glm::vec3& position, float radius,
    const glm::vec3& color
);
