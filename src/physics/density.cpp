#include "density.hpp"
#include "base.hpp"
#include "glm/common.hpp"
#include "klein/point.hpp"
#include "klein/translator.hpp"
#include "utils/math.hpp"

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

const std::vector<glm::ivec3>& Density::nearbyCells(const kln::point& p1
) const {
    auto cell = _cell(p1);
    auto halfSize = static_cast<int>(std::ceil(lookupRadius / gridCellSize));
    auto size = halfSize * 2 + 1;
    float squaredRadius = lookupRadius * lookupRadius;
    glm::vec3 centerPos(p1.x(), p1.y(), p1.z());

    // std::vector<glm::ivec3> cells;
    _cellCache.clear();
    _cellCache.reserve(size * size * size / 2);

    for (int x = -halfSize; x <= halfSize; ++x) {
        for (int y = -halfSize; y <= halfSize; ++y) {
            for (int z = -halfSize; z <= halfSize; ++z) {
                // cells.emplace_back(cell.x + x, cell.y + y, cell.z + z);
                glm::vec3 cellCenter = cellInSpace(glm::ivec3(
                    centerPos.x + x, centerPos.y + y, centerPos.z + z
                ));
                auto diff = cellCenter - centerPos;
                float squaredDist = glm::dot(diff, diff);

                if (squaredDist <= squaredRadius) {
                    _cellCache.emplace_back(
                        centerPos.x + x, centerPos.y + y, centerPos.z + z
                    );
                }
            }
        }
    }
    return _cellCache;
}

const std::vector<Particle*>& Density::nearbyParticles(const kln::point& p1
) const {
    _particleCache.clear();
    // std::vector<Particle*> particles;
    auto cells = nearbyCells(p1);

    // Reserve a reasonable amount based on typical density
    _particleCache.reserve(cells.size() * 4);
    // Assuming ~4 particles per cell on average

    for (const auto& cell : cells) {
        auto range = _particleMap.equal_range(cell);
        for (auto it = range.first; it != range.second; ++it) {
            // auto particle = it->second;
            // particles.push_back(particle);
            _particleCache.push_back(it->second);
        }
    }
    // return particles;
    return _particleCache;
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
    // for (auto& particle : nearbyParticles(p1.position)) {
    //     if (particle == &p1)
    //         continue; // Skip self

    //     float distance = (p1.position & particle->position).norm();
    //     float factor = glm::smoothstep(
    //         repulsionFactor, 0.f, inverseLerp(0.f, lookupRadius, distance)
    //     );
    //     auto direction = (p1.position - particle->position) / distance;
    //     force += kln::translator(
    //         factor, direction.x(), direction.y(), direction.z()
    //     );
    // }

    auto centerCell = _cell(p1.position);
    auto halfSize = static_cast<int>(std::ceil(lookupRadius / gridCellSize));
    float squaredRadius = lookupRadius * lookupRadius;

    for (int x = -halfSize; x <= halfSize; ++x) {
        for (int y = -halfSize; y <= halfSize; ++y) {
            for (int z = -halfSize; z <= halfSize; ++z) {
                auto cell = glm::ivec3(
                    centerCell.x + x, centerCell.y + y, centerCell.z + z
                );
                auto range = _particleMap.equal_range(cell);

                for (auto it = range.first; it != range.second; ++it) {
                    Particle* particle = it->second;
                    if (particle == &p1)
                        continue; // Skip self

                    float distance = (p1.position & particle->position).norm();
                    if (distance <= lookupRadius) {
                        float factor = glm::smoothstep(
                            repulsionFactor, 0.f,
                            inverseLerp(0.f, lookupRadius, distance)
                        );
                        auto direction =
                            (p1.position - particle->position) / distance;
                        force += kln::translator(
                            factor, direction.x(), direction.y(), direction.z()
                        );
                    }
                }
            }
        }
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
