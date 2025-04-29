#include <ranges>

#include "constants.hpp"
#include "creators.hpp"

void drape(
    std::vector<Particle>& particles, std::vector<SpringLink>& links,
    const DrapeParameters& params
) {
    auto [nextCount, mass, knot, anchors, direction] = params;

    for (const auto& i : std::views::iota(0, nextCount)) {
        for (const auto& j : std::views::iota(0, nextCount)) {
            switch (direction) {
            case DrapeDirection::XZ:
                particles.emplace_back(
                    kln::point(
                        (i - int(nextCount / 2)) * knot, 0,
                        (j - int(nextCount / 2)) * knot
                    ),
                    mass
                );
                break;
            case DrapeDirection::XY:
                particles.emplace_back(
                    kln::point(
                        (i - int(nextCount / 2)) * knot,
                        (j - int(nextCount / 2)) * knot, 0
                    ),
                    mass
                );
                break;
            case DrapeDirection::ZY:
                particles.emplace_back(
                    kln::point(
                        0, (j - int(nextCount / 2)) * knot,
                        (i - int(nextCount / 2)) * knot
                    ),
                    mass
                );
                break;
            }
            if (i > 0) {
                links.emplace_back(
                    (i - 1) * nextCount + j, i * nextCount + j, knot
                );
            }
            if (j > 0) {
                links.emplace_back(
                    i * nextCount + (j - 1), i * nextCount + j, knot
                );
            }
            if (i > 0 && j > 0) {
                links.emplace_back(
                    (i - 1) * nextCount + (j - 1), i * nextCount + j,
                    knot * M_SQRT2
                );
                links.emplace_back(
                    i * nextCount + (j - 1), (i - 1) * nextCount + j,
                    knot * M_SQRT2
                );
            }
            switch (anchors) {
            case DrapeAnchors::None: break;
            case DrapeAnchors::Edges:
                if (i == 0 || i == nextCount - 1 || j == 0 ||
                    j == nextCount - 1) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::Corners:
                if ((i == 0 && j == 0) ||
                    (i == nextCount - 1 && j == nextCount - 1) ||
                    (i == 0 && j == nextCount - 1) ||
                    (i == nextCount - 1 && j == 0)) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::Center:
                if (i == nextCount / 2 && j == nextCount / 2) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::TwoCorners:
                if ((i == 0 && j == 0) || (i == 0 && j == nextCount - 1)) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::TwoCorners2:
                if ((i == 0 && j == nextCount - 1) ||
                    (i == nextCount - 1 && j == nextCount - 1)) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::OneEdge:
                if (i == 0) {
                    particles.back().lock = true;
                }
                break;
            case DrapeAnchors::OneEdge2:
                if (j == nextCount - 1) {
                    particles.back().lock = true;
                }
                break;
                break;
            }
        }
    }
}