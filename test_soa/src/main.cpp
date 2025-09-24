#include "test_soa/request.hpp"
#include "test_soa/request_queue_dop.hpp"
#include "test_soa/request_queue_oop.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

struct BenchConfig {
    std::size_t requests = 100'000;
    std::vector<std::size_t> worker_counts = {1, 2, 4};
};

static void run_benchmark_oop(std::size_t requests, std::size_t workers) {
    RequestQueueOOP bench(requests, workers);

    const double setup = bench.setupMs();
    const double proc = bench.processMs();
    const double total = bench.totalMs();

    const double rps_proc = (proc > 0.0) ? (bench.size() * 1000.0 / proc) : 0.0;
    const double rps_total = (total > 0.0) ? (bench.size() * 1000.0 / total) : 0.0;

    std::cout << "[OOP] workers=" << bench.workers() << " requests=" << bench.size() << " setup=" << std::fixed << std::setprecision(2) << setup
              << " ms"
              << " process=" << std::fixed << std::setprecision(2) << proc << " ms"
              << " total=" << std::fixed << std::setprecision(2) << total << " ms"
              << "  thr(process)=" << std::fixed << std::setprecision(2) << rps_proc << " req/s"
              << "  thr(total)=" << std::fixed << std::setprecision(2) << rps_total << " req/s\n";
}

static void run_benchmark_dop(std::size_t requests, std::size_t workers) {
    RequestQueueDOP bench(requests, workers);

    const double setup = bench.setupMs();
    const double proc = bench.processMs();
    const double total = bench.totalMs();

    const double rps_proc = (proc > 0.0) ? (bench.size() * 1000.0 / proc) : 0.0;
    const double rps_total = (total > 0.0) ? (bench.size() * 1000.0 / total) : 0.0;

    std::cout << "[DOP] workers=" << bench.workers() << " requests=" << bench.size() << " setup=" << std::fixed << std::setprecision(2) << setup
              << " ms"
              << " process=" << std::fixed << std::setprecision(2) << proc << " ms"
              << " total=" << std::fixed << std::setprecision(2) << total << " ms"
              << "  thr(process)=" << std::fixed << std::setprecision(2) << rps_proc << " req/s"
              << "  thr(total)=" << std::fixed << std::setprecision(2) << rps_total << " req/s\n";
}

int main() {
    BenchConfig cfg;
    // Adjust as needed:
    cfg.requests = 20'000;
    cfg.worker_counts = {1, 2, 4, 8};

    std::cout << "Benchmarking with " << cfg.requests << " requests\n";
    for (auto w : cfg.worker_counts) {
        run_benchmark_oop(cfg.requests, w);
        run_benchmark_dop(cfg.requests, w);
    }
    return 0;
}
