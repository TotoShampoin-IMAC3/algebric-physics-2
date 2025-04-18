#pragma once

#include "../utils/events.hpp"
#include "Time.hpp"

#include <klein/klein.hpp>

class Particle {
public:
    kln::point position;
    float mass;

    kln::translator velocity {};
    kln::translator force {};
    bool lock {false};

    Event<void(kln::point oldPos, kln::point newPos)> onMove {};

    Particle(const kln::point& position = {}, float mass = 1.f);

    void update(const Second& deltaTime);
    void applyForce(const kln::translator& _force, const Second& deltaTime);
    void prepareForce(const kln::translator& _force);
    void updateForce(const Second& deltaTime);
};

class Link {
public:
    virtual void applyForce(const Second& deltaTime, Particle& p1) = 0;
    virtual void applyForce(
        const Second& deltaTime, Particle& p1, Particle& p2
    ) = 0;

    virtual void prepareForce(Particle& p1) = 0;
    virtual void prepareForce(Particle& p1, Particle& p2) = 0;
};
