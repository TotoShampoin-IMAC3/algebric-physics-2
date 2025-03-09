#pragma once

#include "klein/translator.hpp"
#include <glm/glm.hpp>
#include <klein/klein.hpp>

inline kln::point vecToPoint(const glm::vec3& v) {
    return {v.x, v.y, v.z};
}

inline glm::vec3 pointToVec(const kln::point& p) {
    return {p.x(), p.y(), p.z()};
}

inline kln::translator pointToTranslator(const kln::point& p) {
    // I really don't like that this is the only way to convert a point to a
    // translator. I hope -O2 will do its magic.
    auto l = (p & kln::origin()).norm();
    return kln::translator(l, p.x(), p.y(), p.z());
}
