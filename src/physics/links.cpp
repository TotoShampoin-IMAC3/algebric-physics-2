#include "links.hpp"
#include "../utils/types.hpp"
#include <cassert>
#include <klein/klein.hpp>
#include <stdexcept>

//============================================================================//

Spring::Spring(float length, float stiffness)
    : length(length),
      stiffness(stiffness) {}

void Spring::applyForce(const Second& deltaTime, Particle& p1) {
    throw std::runtime_error("Spring requires two particles");
}

void Spring::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    auto force = _calculateForce(p1.position, p2.position);
    p1.applyForce(force, deltaTime);
    p2.applyForce(force * -1, deltaTime);
}

kln::translator Spring::_calculateForce(
    const kln::point& p1, const kln::point& p2
) {
    auto delta = p1 - p2;
    float distance = (p1 & p2).norm();

    kln::point F = -stiffness * (1 - length / distance) * delta;
    return pointToTranslator(F);
}

//============================================================================//

ConstantForce::ConstantForce(const kln::translator& force)
    : force(force) {}

void ConstantForce::applyForce(const Second& deltaTime, Particle& p1) {
    p1.applyForce(force, deltaTime);
}
void ConstantForce::applyForce(
    const Second& deltaTime, Particle& p1, Particle& p2
) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}

//============================================================================//

Viscosity::Viscosity(float force)
    : force(force) {}

void Viscosity::applyForce(const Second& deltaTime, Particle& p1) {
    p1.applyForce(p1.velocity * -force / p1.mass, deltaTime);
}
void Viscosity::applyForce(
    const Second& deltaTime, Particle& p1, Particle& p2
) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}

//============================================================================//

Wall::Wall(const kln::plane& wall, float force)
    : wall(wall),
      force(force) {}

void Wall::applyForce(const Second& deltaTime, Particle& p1) {
    auto distance = p1.position & wall;
    if (distance.scalar() < 0) {
        p1.applyForce(
            kln::translator(force, wall.e1(), wall.e2(), wall.e3()), deltaTime
        );
    }
}
void Wall::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}
