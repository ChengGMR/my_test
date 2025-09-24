#pragma once

#include "test_soa/request.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <random>
#include <string>
#include <thread>
#include <vector>

class RequestQueueOOP {
   public:
    RequestQueueOOP(std::size_t request_num, std::size_t worker_num) : m_workers(worker_num ? worker_num : 1) {
        const auto t_begin = std::chrono::steady_clock::now();

        // 0) Build workload (excluded from processing time)
        m_requests.resize(request_num);
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist_d(0.0, 100.0);
        std::uniform_int_distribution<long> dist_l(0, 1000);
        for (std::size_t i = 0; i < request_num; ++i) {
            m_requests[i].id = i;
            m_requests[i].message = "msg_" + std::to_string(i);
            m_requests[i].parameter_1 = dist_d(rng);
            m_requests[i].parameter_2 = dist_l(rng);
        }

        // 1) Partition work
        const std::size_t base = request_num / m_workers;
        const std::size_t rem = request_num % m_workers;

        // 2) Launch workers (they wait on 'go')
        std::atomic<std::size_t> ready{0};
        std::atomic<bool> go{false};
        std::vector<std::thread> threads;
        threads.reserve(m_workers);

        std::size_t start = 0;
        for (std::size_t w = 0; w < m_workers; ++w) {
            const std::size_t count = base + (w < rem ? 1 : 0);
            const std::size_t s = start, e = s + count;
            start = e;

            threads.emplace_back([this, s, e, &ready, &go] {
                // Signal ready, then wait for the start signal
                ready.fetch_add(1, std::memory_order_acq_rel);
                while (!go.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }
                for (std::size_t i = s; i < e; ++i) {
                    m_requests[i].processRequest();
                }
            });
        }

        // 3) Finish setup timing when all threads are ready
        while (ready.load(std::memory_order_acquire) < m_workers) {
            std::this_thread::yield();
        }

        const auto t_setup_end = std::chrono::steady_clock::now();

        // 4) Processing window
        const auto t_proc_begin = std::chrono::steady_clock::now();
        go.store(true, std::memory_order_release);

        for (auto& th : threads) {
            th.join();
        }
        const auto t_proc_end = std::chrono::steady_clock::now();

        // 5) Record times
        m_setup_ms = std::chrono::duration<double, std::milli>(t_setup_end - t_begin).count();
        m_process_ms = std::chrono::duration<double, std::milli>(t_proc_end - t_proc_begin).count();
        m_total_ms = std::chrono::duration<double, std::milli>(t_proc_end - t_begin).count();
    }

    // Accessors
    double setupMs() const noexcept { return m_setup_ms; }
    double processMs() const noexcept { return m_process_ms; }
    double totalMs() const noexcept { return m_total_ms; }
    std::size_t workers() const noexcept { return m_workers; }
    std::size_t size() const noexcept { return m_requests.size(); }

   private:
    std::vector<Request> m_requests;
    std::size_t m_workers{1};
    double m_setup_ms{0.0};
    double m_process_ms{0.0};
    double m_total_ms{0.0};
};
