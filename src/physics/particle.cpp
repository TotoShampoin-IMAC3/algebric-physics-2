#include "base.hpp"

Particle::Particle(const kln::point& position, float mass)
    : position(position),
      mass(mass) {}

void Particle::update(const Second& deltaTime) {
    if (lock)
        velocity = {};
    auto oldPosition = position;
    position = (velocity * static_cast<float>(deltaTime))(position);

    onMove(oldPosition, position);
}
void Particle::applyForce(
    const kln::translator& _force, const Second& deltaTime
) {
    auto df = _force * static_cast<float>(deltaTime);
    velocity = velocity * df;
}
void Particle::prepareForce(const kln::translator& _force) {
    force += _force;
}
void Particle::updateForce(const Second& deltaTime) {
    velocity += force * static_cast<float>(deltaTime);
    force = {};
}
