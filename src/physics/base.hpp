#pragma once
#include "Time.hpp"
#include "klein/translator.hpp"
#include <klein/klein.hpp>

class Particle {
public:
    kln::point position;
    float mass;

    kln::translator velocity {};
    kln::translator force {};
    bool lock {false};

    Particle(const kln::point& position = {}, float mass = 1.f)
        : position(position),
          mass(mass) {}

    void update(const Second& deltaTime) {
        if (lock)
            velocity = {};
        position = (velocity * static_cast<float>(deltaTime))(position);
    }
    void applyForce(const kln::translator& _force, const Second& deltaTime) {
        velocity += _force * static_cast<float>(deltaTime);
    }
    void prepareForce(const kln::translator& _force, const Second& deltaTime) {
        force += _force * static_cast<float>(deltaTime);
    }
    void updateForce() {
        velocity += force;
        force = {};
    }
};

class Link {
public:
    virtual void applyForce(const Second& deltaTime, Particle& p1) = 0;
    virtual void applyForce(
        const Second& deltaTime, Particle& p1, Particle& p2
    ) = 0;
};
