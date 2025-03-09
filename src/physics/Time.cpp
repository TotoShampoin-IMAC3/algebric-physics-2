#include "Time.hpp"
#include <utility>

#include <glfwpp/glfwpp.h>

Time::Time() {
    _startTime = glfw::getTime();
    _lastTime = _startTime;
    _deltaTime = 0.0;
}

void Time::tick() {
    auto currentTime = glfw::getTime();
    _deltaTime = currentTime - _lastTime;
    _lastTime = currentTime;
    _elapsedTime = currentTime - _startTime;
}

Second Time::deltaTime() const {
    return _deltaTime;
}

Second Time::elapsedTime() const {
    return _elapsedTime;
}
