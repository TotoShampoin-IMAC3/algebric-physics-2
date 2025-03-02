#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 OrbitCamera::view() const {
    glm::vec3 position = //
        glm::vec3(
            glm::cos(pitch) * glm::cos(yaw) * distance,
            glm::sin(pitch) * distance,
            glm::cos(pitch) * glm::sin(yaw) * distance
        ) +
        target;

    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void OrbitCamera::move(float dx, float dy, float dz) {
    target += glm::vec3(dx, dy, dz);
}

void OrbitCamera::rotate(float dp, float dy) {

    if (pitch + dp * sensitivity < glm::half_pi<float>() &&
        pitch + dp * sensitivity > -glm::half_pi<float>()) {
        pitch += dp * sensitivity;
    }
    yaw += dy * sensitivity;
}

void OrbitCamera::zoom(float dz) {
    distance += dz;
}
