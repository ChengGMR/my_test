#pragma once

#include "test_soa/request_soa.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <random>
#include <string>
#include <thread>
#include <vector>

class RequestQueueDOP {
   public:
    RequestQueueDOP(std::size_t request_num, std::size_t worker_num) : m_data(request_num), m_workers(worker_num ? worker_num : 1) {
        const auto t_begin = std::chrono::steady_clock::now();

        // 0) Initialize SoA with random data (excluded from processing time)
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_real_distribution<double> dist_d(0.0, 100.0);
        std::uniform_int_distribution<long> dist_l(0, 1000);

        for (std::size_t i = 0; i < request_num; ++i) {
            m_data.ids[i] = i;
            m_data.messages[i] = "msg_" + std::to_string(i);
            m_data.param1[i] = dist_d(rng);
            m_data.param2[i] = dist_l(rng);
        }

        // 1) Partition work
        const std::size_t base = request_num / m_workers;
        const std::size_t rem = request_num % m_workers;

        // 2) Launch workers that wait on 'go'
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
                // Signal readiness, then wait for start
                ready.fetch_add(1, std::memory_order_acq_rel);
                while (!go.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }
                for (std::size_t i = s; i < e; ++i) {
                    m_data.process(i); // ensure this does no I/O/sleep for fair timing
                }
            });
        }

        // 3) Finish setup timing once all threads are ready
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

    // Results
    double setupMs() const noexcept { return m_setup_ms; }
    double processMs() const noexcept { return m_process_ms; }
    double totalMs() const noexcept { return m_total_ms; }
    std::size_t workers() const noexcept { return m_workers; }
    std::size_t size() const noexcept { return m_data.ids.size(); }

   private:
    RequestsSoA m_data;
    std::size_t m_workers;
    double m_setup_ms{0.0};
    double m_process_ms{0.0};
    double m_total_ms{0.0};
};
