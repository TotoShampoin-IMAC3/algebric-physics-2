#pragma once

#include <functional>
#include <vector>

template <typename Func>
class Event {
public:
    Event() = default;

    void operator+=(const std::function<Func>& callback) {
        _callbacks.push_back(callback);
    }

    // Calls all callbacks, and ignores the return value
    template <typename... Args>
    void operator()(Args... args) const
        requires(std::is_invocable_v<Func, Args...>)
    {
        for (auto& callback : _callbacks) {
            (void)callback(args...);
        }
    }

private:
    std::vector<std::function<Func>> _callbacks;
};
