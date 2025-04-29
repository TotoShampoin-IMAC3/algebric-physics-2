#pragma once

#include "base.hpp"
#include "klein/translator.hpp"

class Spring : public Link {
public:
    float length;
    float stiffness;
    float viscosity;

    Spring(float length, float stiffness, float viscosity);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

    void prepareForce(Particle& p1) override;
    void prepareForce(Particle& p1, Particle& p2) override;

private:
    kln::translator _calculateForce(Particle& p1, Particle& p2);
};

class ConstantForce : public Link {
public:
    kln::translator force;

    ConstantForce(const kln::translator& force);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

    void prepareForce(Particle& p1) override;
    void prepareForce(Particle& p1, Particle& p2) override;
};

class Wall : public Link {
public:
    kln::plane wall;
    float force;

    Wall(const kln::plane& wall, float force);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

    void prepareForce(Particle& p1) override;
    void prepareForce(Particle& p1, Particle& p2) override;

private:
    kln::translator _calculateForce(Particle& p1);
};

class Wind : public Link {
public:
    kln::point frequency;
    kln::point amplitude;

    Wind(const kln::point& frequency, const kln::point& amplitude);

    void applyForce(const Second& deltaTime, Particle& p1) override;
    void applyForce(const Second& deltaTime, Particle& p1, Particle& p2)
        override;

    void prepareForce(Particle& p1) override;
    void prepareForce(Particle& p1, Particle& p2) override;

    void update(float deltaTime);

private:
    kln::translator _calculateForce(Particle& p1);
    float _time;
};
