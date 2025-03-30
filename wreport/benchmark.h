#ifndef WREPORT_BENCHMARK_H
#define WREPORT_BENCHMARK_H

/** @file
 * Simple benchmark infrastructure.
 */

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace wreport {
namespace benchmark {

struct Benchmark;

/// Collect timings for one task
struct Task
{
    // Unmanaged pointer to the benchmark we belong to
    Benchmark* parent;
    // Name of this task
    std::string name;
    // Number of time this task has run
    unsigned run_count = 0;
    // Total user time
    clock_t utime      = 0;
    // Total system time
    clock_t stime      = 0;

    Task(Benchmark* parent, const std::string& name);

    // Run the given function and collect timings for it
    void collect(std::function<void()> f);
};

/// Notify of progress during benchmark execution
struct Progress
{
    virtual ~Progress() {}

    virtual void start_benchmark(const Benchmark& b)                = 0;
    virtual void end_benchmark(const Benchmark& b)                  = 0;
    virtual void start_iteration(const Benchmark& b, unsigned cur,
                                 unsigned total)                    = 0;
    virtual void end_iteration(const Benchmark& b, unsigned cur,
                               unsigned total)                      = 0;
    virtual void test_failed(const Benchmark& b, std::exception& e) = 0;
};

/**
 * Basic progress implementation writing progress information to the given
 * output stream
 */
struct BasicProgress : Progress
{
    FILE* out;
    FILE* err;

    BasicProgress(FILE* out = stdout, FILE* err = stderr);

    void start_benchmark(const Benchmark& b) override;
    void start_iteration(const Benchmark& b, unsigned cur,
                         unsigned total) override;
    void end_iteration(const Benchmark& b, unsigned cur,
                       unsigned total) override;
    void end_benchmark(const Benchmark& b) override;
    void test_failed(const Benchmark& b, std::exception& e) override;
};

/**
 * Base class for all benchmarks.
 */
struct Benchmark
{
    // Name of this benchmark
    std::string name;
    // Number of repetitions
    unsigned repetitions = 10;
    // Unmanaged pointers to the tasks in this benchmark
    std::vector<Task*> tasks;
    // Main task, collecting timings for the toplevel run
    Task task_main;

    Benchmark(const std::string& name);
    virtual ~Benchmark();

    /**
     * Set up the environment for this benchmark.
     *
     * This is run outside of timings. By default it does nothing.
     */
    virtual void setup_main() {}

    /**
     * Tear down the environment for this benchmark.
     *
     * This is run outside of timings. By default it does nothing.
     */
    virtual void teardown_main() {}

    /**
     * Set up the environment for an iteration of this benchmark.
     *
     * This is run outside of timings. By default it does nothing.
     */
    virtual void setup_iteration() {}

    /**
     * Tear down the environment for an iteration of this benchmark.
     *
     * This is run outside of timings. By default it does nothing.
     */
    virtual void teardown_iteration() {}

    /// Run the benchmark and collect timings
    void run(Progress& progress);

    /// Print timings to stdout
    void print_timings();

    /// Main body of this benchmark
    virtual void main() = 0;
};

/// Collect all existing benchmarks
struct Registry
{
    std::vector<Benchmark*> benchmarks;

    /// Add a benchmark to this registry
    void add(Benchmark* b);

    /**
     * Get the static instance of the registry
     */
    static Registry& get();

    /**
     * Basic implementation of a main function that runs all benchmarks linked
     * into the program. This allows to make a benchmark runner tool with just
     * this code:
     *
     * \code
     * #include <wreport/benchmark.h>
     *
     * int main (int argc, const char* argv[])
     * {
     *     wreport::benchmark::Registry::basic_run(argc, argv);
     * }
     * \endcode
     *
     * If you need different logic in your benchmark running code, you can use
     * the source code of basic_run as a template for writing your own.
     */
    static void basic_run(int argc, const char* argv[]);
};

} // namespace benchmark
} // namespace wreport

#endif
