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

Profiler::Profiler() {
    _times.reserve(1000);
    begin();
}

void Profiler::begin() {
    _times.clear();
    _times.push_back(glfw::getTime());
}

void Profiler::tick() {
    _times.push_back(glfw::getTime());
}

Second Profiler::operator[](size_t index) const {
    if (index > _times.size()) {
        return 0.0;
    }
    return _times[index + 1] - _times[index];
}
