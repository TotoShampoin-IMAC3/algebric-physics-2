#pragma once

#include <glm/glm.hpp>
#include <klein/klein.hpp>

kln::point vecToPoint(const glm::vec3& v);
glm::vec3 pointToVec(const kln::point& p);

kln::translator pointToTranslator(const kln::point& p);
