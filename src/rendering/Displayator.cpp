#include "Displayator.hpp"
#include "../utils/types.hpp"
#include "gl/helper.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "klein/line.hpp"
#include "klein/plane.hpp"
#include "klein/point.hpp"
#include "klein/translator.hpp"
#include <cassert>
#include <execution>

Shader& basicFrag() {
    static auto shader = loadShader(GL_FRAGMENT_SHADER, "res/basic.frag");
    return shader;
}

Displayator::Displayator()
    : _pointShader(std::move(
          Program()
              .attachShader(loadShader(GL_VERTEX_SHADER, "res/point.vert"))
              .attachShader(loadShader(GL_FRAGMENT_SHADER, "res/point.frag"))
              .link()
      )),
      _lineShader(std::move(
          Program()
              .attachShader(loadShader(GL_VERTEX_SHADER, "res/line.vert"))
              .attachShader(basicFrag())
              .link()
      )),
      _planeShader(std::move(
          Program()
              .attachShader(loadShader(GL_VERTEX_SHADER, "res/plane.vert"))
              .attachShader(loadShader(GL_FRAGMENT_SHADER, "res/plane.frag"))
              .link()
      )),
      _pointsShader(std::move(
          Program()
              .attachShader(loadShader(GL_VERTEX_SHADER, "res/points.vert"))
              .attachShader(basicFrag())
              .link()
      )),
      _linesShader(std::move(
          Program()
              .attachShader(loadShader(GL_VERTEX_SHADER, "res/lines.vert"))
              .attachShader(basicFrag())
              .link()
      )),
      _quadMesh(
          {
              {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
              { {1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
              {  {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
              { {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
},
          {0, 1, 2, 0, 2, 3}
      ) {}

Displayator& Displayator::drawPoint(const glm::vec3& position) {
    _pointShader.use()
        .setUniform("color", _color)
        .setUniform("size", _pointSize)
        .setUniform("point", position)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind().draw().unbind();
    return *this;
}

Displayator& Displayator::drawLine(
    const glm::vec3& start, const glm::vec3& end
) {
    _lineShader.use()
        .setUniform("color", _color)
        .setUniform("width", _lineWidth)
        .setUniform("start", start)
        .setUniform("end", end)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind().draw().unbind();
    return *this;
}

Displayator& Displayator::drawPlane(
    const glm::vec3& position, const glm::vec3& normal
) {
    _planeShader.use()
        .setUniform("color", _color)
        .setUniform("position", position)
        .setUniform("normal", normal)
        .setUniform("size", _planeSize)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind().draw().unbind();
    return *this;
}

Displayator& Displayator::drawPoint(const kln::point& point) {
    return drawPoint(pointToVec(point));
}

Displayator& Displayator::drawLine(
    const kln::point& start, const kln::point& end
) {
    return drawLine(pointToVec(start), pointToVec(end));
}

Displayator& Displayator::drawLine(
    const kln::line& line, float length, const kln::point& from,
    bool doubleSided
) {
    auto fromP = (from | line) ^ line;
    auto T = kln::translator(-length, line.e23(), line.e31(), line.e12());
    auto start = doubleSided ? -T(fromP) : fromP;
    auto end = T(fromP);
    return drawLine(start, end);
}

Displayator& Displayator::drawPlane(const kln::plane& plane) {
    auto position = (kln::origin() | plane) ^ plane;
    auto normal = kln::point(plane.e1(), plane.e2(), plane.e3());
    return drawPlane(pointToVec(position), pointToVec(normal));
}

Displayator& Displayator::drawPoints(const std::vector<glm::vec3>& positions) {
    _pointsShader.use()
        .setUniform("color", _color)
        .setUniform("size", _pointSize)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind();
    std::vector<Instance> instances(positions.size());
    std::transform(
        std::execution::par_unseq, positions.begin(), positions.end(),
        instances.begin(),
        [](const glm::vec3& position) {
            return Instance {position, glm::vec3(0.0f)};
        }
    );
    _quadMesh.drawInstanced(instances);
    _quadMesh.unbind();
    return *this;
}

Displayator& Displayator::drawLines(
    const std::vector<glm::vec3>& starts, const std::vector<glm::vec3>& ends
) {
    _linesShader.use()
        .setUniform("color", _color)
        .setUniform("width", _lineWidth)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind();
    std::vector<Instance> instances(starts.size());
    std::transform(
        std::execution::par_unseq, starts.begin(), starts.end(), ends.begin(),
        instances.begin(),
        [](const glm::vec3& start, const glm::vec3& end) {
            return Instance {start, end};
        }
    );
    _quadMesh.drawInstanced(instances);
    _quadMesh.unbind();
    return *this;
}

Displayator& Displayator::drawLines(
    const std::vector<std::pair<glm::vec3, glm::vec3>>& lines
) {
    _linesShader.use()
        .setUniform("color", _color)
        .setUniform("width", _lineWidth)
        .setUniform("view", _view)
        .setUniform("projection", _projection);
    _quadMesh.bind();
    std::vector<Instance> instances(lines.size());
    std::transform(
        std::execution::par_unseq, lines.begin(), lines.end(),
        instances.begin(),
        [](const std::pair<glm::vec3, glm::vec3>& line) {
            return Instance {line.first, line.second};
        }
    );
    _quadMesh.drawInstanced(instances);
    _quadMesh.unbind();
    return *this;
}

Displayator& Displayator::setView(const glm::mat4& view) {
    this->_view = view;
    return *this;
}

Displayator& Displayator::setProjection(const glm::mat4& projection) {
    this->_projection = projection;
    return *this;
}

Displayator& Displayator::setProjection(
    float fov, float aspect, float near, float far
) {
    this->_projection = glm::perspective(fov, aspect, near, far);
    return *this;
}

Displayator& Displayator::setColor(const glm::vec3& color) {
    this->_color = color;
    return *this;
}

Displayator& Displayator::setPointSize(float size) {
    this->_pointSize = size;
    return *this;
}

Displayator& Displayator::setLineWidth(float width) {
    this->_lineWidth = width;
    return *this;
}

Displayator& Displayator::setPlaneSize(float size) {
    this->_planeSize = size;
    return *this;
}
