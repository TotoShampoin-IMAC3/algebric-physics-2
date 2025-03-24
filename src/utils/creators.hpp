#pragma once

#include "physics/base.hpp"
#include "physics/links.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct SpringLink {
    int a;
    int b;
    float length;
};

enum class DrapeAnchors {
    None,
    Corners,
    Edges,
    Center,
};
inline std::string to_string(DrapeAnchors anchors) {
    switch (anchors) {
    case DrapeAnchors::None: return "None";
    case DrapeAnchors::Corners: return "Corners";
    case DrapeAnchors::Edges: return "Edges";
    case DrapeAnchors::Center: return "Center";
    }
    return "Unknown";
}
const auto drape_anchors = {
    DrapeAnchors::None,
    DrapeAnchors::Corners,
    DrapeAnchors::Edges,
    DrapeAnchors::Center,
};

struct DrapeParameters {
    int n;
    float mass;
    float spread;
    DrapeAnchors anchors = DrapeAnchors::Corners;
};

void drape(
    std::vector<Particle>& particles, std::vector<SpringLink>& links,
    const DrapeParameters& grid
);
