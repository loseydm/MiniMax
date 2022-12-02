#ifndef BOOK_TIMER_HPP
#define BOOK_TIMER_HPP

#include <chrono>
#include <iostream>

struct Stopwatch {
    Stopwatch() {
        start = std::chrono::high_resolution_clock::now();
    }

    [[nodiscard]] std::chrono::nanoseconds measure() const {
        return std::chrono::high_resolution_clock::now() - start;
    }

    [[maybe_unused]] void reset() {
        start = std::chrono::high_resolution_clock::now();
    }

    friend std::ostream &operator<<(std::ostream &out, const Stopwatch &watch) {
        auto ns = watch.measure().count();

        out << ns / 1E9F << " seconds";

        return out;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#endif

