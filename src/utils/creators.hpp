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
    TwoCorners,
    TwoCorners2,
    OneEdge,
    OneEdge2,
};
enum class DrapeDirection {
    XZ,
    XY,
    ZY,
};
inline std::string to_string(DrapeAnchors anchors) {
    switch (anchors) {
    case DrapeAnchors::None: return "None";
    case DrapeAnchors::Corners: return "Corners";
    case DrapeAnchors::Edges: return "Edges";
    case DrapeAnchors::Center: return "Center";
    case DrapeAnchors::TwoCorners: return "Two Corners";
    case DrapeAnchors::TwoCorners2: return "Two Corners 2";
    case DrapeAnchors::OneEdge: return "One Edge";
    case DrapeAnchors::OneEdge2: return "One Edge 2";
    }
    return "Unknown";
}
const auto drape_anchors = {
    DrapeAnchors::None,    DrapeAnchors::Corners,    DrapeAnchors::Edges,
    DrapeAnchors::Center,  DrapeAnchors::TwoCorners, DrapeAnchors::TwoCorners2,
    DrapeAnchors::OneEdge, DrapeAnchors::OneEdge2,
};

struct DrapeParameters {
    int n;
    float mass;
    float spread;
    DrapeAnchors anchors = DrapeAnchors::Corners;
    DrapeDirection direction = DrapeDirection::XZ;
};

void drape(
    std::vector<Particle>& particles, std::vector<SpringLink>& links,
    const DrapeParameters& grid
);
