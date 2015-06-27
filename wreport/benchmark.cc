#include "benchmark.h"
#include <iomanip>
#include <iostream>
#include <sys/times.h>
#include <unistd.h>

using namespace std;

namespace {
double ticks_per_sec = sysconf(_SC_CLK_TCK);

}

namespace wreport {
namespace benchmark {

Task::Task(Benchmark* parent, const std::string& name)
    : parent(parent), name(name)
{
    parent->tasks.push_back(this);
}

void Task::collect(std::function<void()> f)
{
    run_count += 1;

    struct tms tms_start, tms_end;
    times(&tms_start);
    f();
    times(&tms_end);

    utime += tms_end.tms_utime - tms_start.tms_utime;
    stime += tms_end.tms_stime - tms_start.tms_stime;
}

void Registry::add(Benchmark* b)
{
    benchmarks.push_back(b);
}

Registry& Registry::get()
{
    static Registry* registry = 0;
    if (!registry)
        registry = new Registry();
    return *registry;
}

Benchmark::Benchmark(const std::string& name)
    : name(name), task_main(this, "main")
{
    Registry::get().add(this);
}
Benchmark::~Benchmark() {}

void Benchmark::run(Progress& progress)
{
    progress.start_benchmark(*this);
    setup_main();

    for (unsigned i = 0; i < repetitions; ++i)
    {
        progress.start_iteration(*this, i, repetitions);
        setup_iteration();
        task_main.collect([&]() { main(); });
        teardown_iteration();
        progress.end_iteration(*this, i, repetitions);
    }

    teardown_main();
    progress.end_benchmark(*this);
}

void Benchmark::print_timings()
{
    double ticks_per_second = (double)sysconf(_SC_CLK_TCK);

    for (auto& t: tasks)
    {
        fprintf(stdout, "%s.%s: %d runs, user: %.2fs (%.1f%%), sys: %.2fs (%.1f%%), total: %.2fs (%.1f%%)\n",
                name.c_str(),
                t->name.c_str(),
                t->run_count,
                t->utime / ticks_per_sec,
                t->utime * 100.0 / task_main.utime,
                t->stime / ticks_per_sec,
                t->stime * 100.0 / task_main.stime,
                (t->utime + t->stime) / ticks_per_sec,
                (t->utime + t->stime) * 100.0 / (task_main.utime + task_main.stime));
    }
}

BasicProgress::BasicProgress(FILE* out, FILE* err)
    : out(out), err(err) {}

void BasicProgress::start_benchmark(const Benchmark& b)
{
    fprintf(out, "%s: starting... ", b.name.c_str());
    fflush(out);
}
void BasicProgress::start_iteration(const Benchmark& b, unsigned cur, unsigned total)
{
    fprintf(out, "\r%s: iteration %u/%u...    ", b.name.c_str(), cur + 1, total);
    fflush(out);
}
void BasicProgress::end_iteration(const Benchmark& b, unsigned cur, unsigned total)
{
    fprintf(out, "\r%s: iteration %u/%u done.", b.name.c_str(), cur + 1, total);
    fflush(out);
}
void BasicProgress::end_benchmark(const Benchmark& b)
{
    fprintf(out, "\r%s: done.                   \r", b.name.c_str());
    fflush(out);
}
void BasicProgress::test_failed(const Benchmark& b, std::exception& e)
{
    fprintf(err, "\n%s: benchmark failed: %s\n", b.name.c_str(), e.what());
}

void Registry::basic_run(int argc, const char* argv[])
{
    BasicProgress progress;

    // Run all benchmarks
    for (auto& b: get().benchmarks)
    {
        try {
            b->run(progress);
        } catch (std::exception& e) {
            progress.test_failed(*b, e);
            continue;
        }
        b->print_timings();
    }
}

#if 0
void Runner::dump_csv(std::ostream& out)
{
    out << "Suite,Test,User,System" << endl;
    for (auto l : log)
    {
        out << l.b_name << "," << l.name << "," << l.utime << "," << l.stime << endl;
    }
}
#endif

}
}
