#pragma once

#include "physics/base.hpp"
#include "physics/links.hpp"
#include <glm/glm.hpp>
#include <vector>

struct SpringLink {
    int a;
    int b;
    float length;
};

struct DrapeParameters {
    int n;
    float mass;
    float spread;
};

void drape(
    std::vector<Particle>& particles, std::vector<SpringLink>& links,
    const DrapeParameters& grid
);
