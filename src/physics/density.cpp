#include "density.hpp"
#include "base.hpp"
#include "klein/point.hpp"
#include "klein/translator.hpp"

void Density::setParticles(std::vector<Particle>& particles) {
    _particleMap.clear();
    _particleMap.reserve(particles.size());
    for (auto& p : particles) {
        _particleMap.emplace(_cell(p), &p);
        p.onMove += [&](const kln::point& oldPos, const kln::point& newPos) {
            auto oldHash = _cell(oldPos);
            auto newHash = _cell(newPos);

            if (oldHash != newHash) {
                auto range = _particleMap.equal_range(oldHash);
                for (auto it = range.first; it != range.second; ++it) {
                    if (it->second == &p) {
                        _particleMap.erase(it);
                        break;
                    }
                }
                _particleMap.emplace(newHash, &p);
            }
        };
    }
}

std::vector<glm::ivec3> Density::nearbyCells(const kln::point& p1) const {
    std::vector<glm::ivec3> cells;
    auto cell = _cell(p1);
    auto halfSize = static_cast<int>(std::ceil(lookupRadius / gridCellSize));
    for (int x = -halfSize; x <= halfSize; ++x) {
        for (int y = -halfSize; y <= halfSize; ++y) {
            for (int z = -halfSize; z <= halfSize; ++z) {
                cells.emplace_back(cell.x + x, cell.y + y, cell.z + z);
            }
        }
    }
    return cells;
}

std::vector<Particle*> Density::nearbyParticles(const kln::point& p1) const {
    std::vector<Particle*> particles;
    auto cells = nearbyCells(p1);
    for (const auto& cell : cells) {
        auto range = _particleMap.equal_range(cell);
        for (auto it = range.first; it != range.second; ++it) {
            auto particle = it->second;
            // // if (kln::distance(p1, particle->position) < lookupRadius) {
            // if ((p1 & particle->position).norm() < lookupRadius) {
            //     particles.push_back(particle);
            // }
            particles.push_back(particle);
        }
    }
    return particles;
}

void Density::applyForce(const Second& deltaTime, Particle& p1) {
    p1.applyForce(_calculateForce(p1), deltaTime);
}
void Density::applyForce(const Second& deltaTime, Particle& p1, Particle& p2) {
    applyForce(deltaTime, p1);
    applyForce(deltaTime, p2);
}

void Density::prepareForce(Particle& p1) {
    p1.prepareForce(_calculateForce(p1));
}
void Density::prepareForce(Particle& p1, Particle& p2) {
    prepareForce(p1);
    prepareForce(p2);
}

kln::translator Density::_calculateForce(const Particle& p1) const {
    kln::translator force = {};
    for (auto& particle : nearbyParticles(p1.position)) {
        if (particle == &p1)
            continue; // Skip self

        float distance = (p1.position & particle->position).norm();
        // TODO
    }

    return force;
}

glm::ivec3 Density::_cell(const kln::point& p1) const {
    return glm::ivec3(
        static_cast<int>(std::round(p1.x() / gridCellSize)),
        static_cast<int>(std::round(p1.y() / gridCellSize)),
        static_cast<int>(std::round(p1.z() / gridCellSize))
    );
}
glm::ivec3 Density::_cell(const Particle& p1) const {
    return _cell(p1.position);
}
