#pragma once

#include <vector>
using Second = double;

class Time {
public:
    Time();

    void tick();

    Second deltaTime() const;
    Second elapsedTime() const;

private:
    Second _startTime;
    Second _lastTime;
    Second _deltaTime;
    Second _elapsedTime;
};

class Profiler {
public:
    Profiler();

    void begin();
    void tick();

    Second operator[](size_t index) const;

private:
    std::vector<Second> _times;
};
