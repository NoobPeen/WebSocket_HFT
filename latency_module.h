#ifndef LATENCY_MODULE_H
#define LATENCY_MODULE_H

#include <chrono>
#include <string>

class LatencyModule {
public:
    // Start a timer
    static std::chrono::high_resolution_clock::time_point start();

    // End the timer and calculate the latency
    static void end(const std::chrono::high_resolution_clock::time_point& start_time, const std::string& action_name);
};

#endif // LATENCY_MODULE_H
