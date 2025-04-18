#pragma once

#include "base.hpp"
#include "links.hpp"
#include <glm/glm.hpp>
#include <klein/klein.hpp>
#include <unordered_map>

// Required for unordered_map to work with glm types
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Density : public Link {
public:
    Density() = default;
    Density(float repulsionFactor, float lookupRadius, float gridCellSize)
        : repulsionFactor(repulsionFactor),
          lookupRadius(lookupRadius),
          gridCellSize(gridCellSize) {}

    float repulsionFactor = 1.f; // Factor to control the repulsion force
    float lookupRadius = 1.f;    // Radius for looking up particles in the grid
    float gridCellSize = 1.f;    // Size of the grid cell for spatial hashing

    void setParticles(std::vector<Particle>&);
    const std::vector<glm::ivec3>& nearbyCells(const kln::point& p1) const;
    const std::vector<Particle*>& nearbyParticles(const kln::point& p1) const;

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

    void prepareForce(Particle& p1) override;
    void prepareForce(Particle& p1, Particle& p2) override;

    glm::ivec3 cell(const kln::point& p1) const { return _cell(p1); }
    glm::vec3 cellInSpace(const kln::point& p1) const {
        return glm::vec3(_cell(p1)) * gridCellSize;
    }
    glm::vec3 cellInSpace(const glm::ivec3& c) const {
        return glm::vec3(c) * gridCellSize;
    }

private:
    using uint = unsigned int;
    std::unordered_multimap<glm::ivec3, Particle*> _particleMap {};

    mutable std::vector<glm::ivec3> _cellCache;
    mutable std::vector<Particle*> _particleCache;

    kln::translator _calculateForce(const Particle& p1) const;

    glm::ivec3 _cell(const kln::point& p1) const;
    glm::ivec3 _cell(const Particle& p1) const;
};
