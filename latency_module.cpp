#include "latency_module.h"
#include <iostream>

std::chrono::high_resolution_clock::time_point LatencyModule::start() {
    return std::chrono::high_resolution_clock::now();
}

void LatencyModule::end(const std::chrono::high_resolution_clock::time_point& start_time, const std::string& action_name) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> latency = end_time - start_time;
    std::cout << action_name << " Latency: " << latency.count() << " seconds" << std::endl;
}
