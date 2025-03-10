#pragma once

#include <glm/glm.hpp>

struct OrbitCamera {
    OrbitCamera(float distance = 5.0f, float pitch = 0.0f, float yaw = 0.0f)
        : distance(distance),
          pitch(pitch),
          yaw(yaw) {}

    glm::mat4 view() const;

    void move(float dx, float dy, float dz);
    void rotate(float dp, float dy);
    void zoom(float dz);

    float sensitivity = 0.01f;

    float distance = 5.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    glm::vec3 target {0.0f, 0.0f, 0.0f};
};
