#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <vector>

struct RequestsSoA {
    std::vector<std::size_t> ids;
    std::vector<std::string> messages;
    std::vector<double> param1;
    std::vector<long> param2;

    RequestsSoA(std::size_t max_requests) {
        ids.resize(max_requests);
        messages.resize(max_requests);
        param1.resize(max_requests);
        param2.resize(max_requests);
    }

    void process(std::size_t i) const {
        static volatile std::size_t sink_sz = 0;
        static volatile double sink_d = 0.0;
        static volatile long sink_l = 0;

        sink_sz += ids[i];
        sink_sz += messages[i].size(); // touches the string object
        sink_d += param1[i];
        sink_l += param2[i];
        // (void) to silence unused warnings if needed
        (void)sink_sz;
        (void)sink_d;
        (void)sink_l;
    }
};
