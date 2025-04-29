#include "links.hpp"
#include "klein/translator.hpp"
#include "utils/types.hpp"
#include <klein/klein.hpp>
#include <stdexcept>

//============================================================================//

Spring::Spring(float length, float stiffness, float viscosity)
    : length(length),
      stiffness(stiffness),
      viscosity(viscosity) {}

void Spring::applyForce(const Second& deltaTime, Particle& p1) {
    throw std::runtime_error("Spring requires two particles");
}

void Spring::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    auto F = _calculateForce(p1, p2);
    p1.applyForce(F / p1.mass, deltaTime);
    p2.applyForce(F / p2.mass * -1, deltaTime);
}

void Spring::prepareForce(Particle& p1) {
    throw std::runtime_error("Spring requires two particles");
}

void Spring::prepareForce(Particle& p1, Particle& p2) {
    auto F = _calculateForce(p1, p2);
    p1.prepareForce(F / p1.mass);
    p2.prepareForce(F / p2.mass * -1);
}

kln::translator Spring::_calculateForce(Particle& p1, Particle& p2) {
    if (&p1 == &p2)
        throw std::runtime_error("Spring cannot be applied to the same particle"
        );
    float k = stiffness;
    float l0 = length;
    float d = (p1.position & p2.position).norm();
    auto M1M2 = p2.position - p1.position;
    auto dir = M1M2 / d;

    if (d == 0)
        return {};

    auto springForce =
        kln::translator(k * (1 - l0 / d), dir.x(), dir.y(), dir.z());
    auto viscousForce = viscosity * (p2.velocity - p1.velocity);

    return springForce * viscousForce;
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

void ConstantForce::prepareForce(Particle& p1) {
    p1.prepareForce(force);
}

void ConstantForce::prepareForce(Particle& p1, Particle& p2) {
    prepareForce(p1);
    prepareForce(p2);
}

//============================================================================//

Wall::Wall(const kln::plane& wall, float force)
    : wall(wall),
      force(force) {}

kln::translator Wall::_calculateForce(Particle& p1) {
    auto distance = p1.position & wall;
    if (distance.scalar() >= 0) {
        return {};
    }
    return kln::translator(force, wall.e1(), wall.e2(), wall.e3()) *
           -distance.scalar();
}

void Wall::applyForce(const Second& deltaTime, Particle& p1) {
    auto F = _calculateForce(p1);
    p1.applyForce(F, deltaTime);
}
void Wall::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}

void Wall::prepareForce(Particle& p1) {
    auto F = _calculateForce(p1);
    p1.prepareForce(F);
}

void Wall::prepareForce(Particle& p1, Particle& p2) {
    prepareForce(p1);
    prepareForce(p2);
}

//==============================================================================

Wind::Wind(const kln::point& frequency, const kln::point& amplitude)
    : frequency(frequency),
      amplitude(amplitude),
      _time(0) {}

void Wind::applyForce(const Second& deltaTime, Particle& p1) {
    auto force = _calculateForce(p1);
    p1.applyForce(force, deltaTime);
}
void Wind::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}

void Wind::prepareForce(Particle& p1) {
    auto force = _calculateForce(p1);
    p1.prepareForce(force);
}

void Wind::prepareForce(Particle& p1, Particle& p2) {
    prepareForce(p1);
    prepareForce(p2);
}

void Wind::update(float deltaTime) {
    _time += deltaTime;
}

kln::translator Wind::_calculateForce(Particle& p1) {
    return pointToTranslator(kln::point(
        amplitude.x() * std::cos(frequency.x() * M_PI * 2 * _time),
        amplitude.y() * std::cos(frequency.y() * M_PI * 2 * _time),
        amplitude.z() * std::cos(frequency.z() * M_PI * 2 * _time)
    ));
}
