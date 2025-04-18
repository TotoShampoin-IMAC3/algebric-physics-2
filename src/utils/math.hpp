#pragma once

template <typename T>
inline T inverseLerp(T a, T b, T value) {
    if (a == b)
        return 0; // Avoid division by zero
    // Clamp the value between a and b
    if (value <= a)
        return 0;
    if (value >= b)
        return 1;
    return (value - a) / (b - a);
}
