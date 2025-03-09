#include "types.hpp"

kln::point vecToPoint(const glm::vec3& v) {
    return {v.x, v.y, v.z};
}

glm::vec3 pointToVec(const kln::point& p) {
    return {p.x(), p.y(), p.z()};
}

kln::translator pointToTranslator(const kln::point& p) {
    // I really don't like that this is the only way to convert a point to a
    // translator. I hope -O2 will do its magic.
    auto l = (p & kln::origin()).norm();
    if (l == 0)
        return {};
    return kln::translator(l, p.x(), p.y(), p.z());
}
