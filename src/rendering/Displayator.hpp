#pragma once

#include "gl/mesh.hpp"
#include "gl/shaders.hpp"

#include <glm/glm.hpp>
#include <klein/klein.hpp>

class Displayator {
public:
    Displayator();

    Displayator& drawPoint(const glm::vec3& position);
    Displayator& drawLine(const glm::vec3& start, const glm::vec3& end);
    Displayator& drawPlane(const glm::vec3& position, const glm::vec3& normal);

    Displayator& drawPoint(const kln::point& point);
    Displayator& drawLine(const kln::point& start, const kln::point& end);
    Displayator& drawLine(
        const kln::line& line, float length = 1000.f,
        const kln::point& from = kln::origin(), bool doubleSided = false
    );
    Displayator& drawPlane(const kln::plane& plane);

    Displayator& drawPoints(const std::vector<glm::vec3>& positions);
    Displayator& drawLines(
        const std::vector<glm::vec3>& starts, const std::vector<glm::vec3>& ends
    );

    Displayator& setView(const glm::mat4& view);
    Displayator& setProjection(const glm::mat4& projection);
    Displayator& setProjection(float fov, float aspect, float near, float far);
    Displayator& setColor(const glm::vec3& color);
    Displayator& setPointSize(float size);
    Displayator& setLineWidth(float width);
    Displayator& setPlaneSize(float size);

private:
    Program _pointShader;
    Program _lineShader;
    Program _planeShader;
    Mesh _quadMesh;

    glm::vec3 _color {1.0f, 1.0f, 1.0f};
    float _pointSize = 1.0f;
    float _lineWidth = 1.0f;
    float _planeSize = 1.0f;
    glm::mat4 _view = glm::mat4(1.0f);
    glm::mat4 _projection = glm::mat4(1.0f);
};
