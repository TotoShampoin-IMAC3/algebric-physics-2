#include "Displayator.hpp"
#include "gl/helper.hpp"

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
    _planeShader.use();
    _planeShader.setUniform("color", _color);
    _planeShader.setUniform("position", position);
    _planeShader.setUniform("normal", normal);
    _planeShader.setUniform("size", _planeSize);
    _planeShader.setUniform("view", _view);
    _planeShader.setUniform("projection", _projection);
    _quadMesh.bind().draw().unbind();
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
