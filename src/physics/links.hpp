#pragma once

#include "base.hpp"

class Spring : public Link {
public:
    float length;
    float stiffness;

    Spring(float length, float stiffness);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

private:
    kln::translator _calculateForce(const kln::point& p1, const kln::point& p2);
};

class ConstantForce : public Link {
public:
    kln::translator force;

    ConstantForce(const kln::translator& force);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;
};

class Viscosity : public Link {
public:
    float force;

    Viscosity(float force);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;
};

class Wall : public Link {
public:
    kln::plane wall;
    float force;

    Wall(const kln::plane& wall, float force);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;
};
