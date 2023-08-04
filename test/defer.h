#pragma once

#include <functional>

class Defer {
public:
    explicit Defer(std::function<void()> f) : func(std::move(f)) {}

    ~Defer() {
        func();
    }

    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;
    Defer(Defer&&) = delete;
    Defer& operator=(Defer&&) = delete;

private:
    std::function<void()> func;
};
