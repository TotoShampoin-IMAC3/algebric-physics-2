#pragma once

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
