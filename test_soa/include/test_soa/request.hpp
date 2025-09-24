#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

struct Request {
    std::size_t id;
    std::string message;
    double parameter_1;
    long parameter_2;

    void processRequest() {
        static volatile std::size_t sink_sz = 0;
        static volatile double sink_d = 0.0;
        static volatile long sink_l = 0;

        sink_sz += id;
        sink_sz += message.size(); // touches the string object
        sink_d += parameter_1;
        sink_l += parameter_2;
        // (void) to silence unused warnings if needed
        (void)sink_sz;
        (void)sink_d;
        (void)sink_l;
    }
};
