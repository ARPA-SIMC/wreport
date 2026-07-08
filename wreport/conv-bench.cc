#include "benchmark.h"
#include "conv.h"
#include <cstdlib>
#include <vector>

using namespace wreport;
using namespace wreport::benchmark;
using namespace std;

namespace {

struct ConvBenchmark : Benchmark
{
    Task conv_identity;
    Task conv_identity_longname;
    Task conv_linear;
    Task conv_linear_longname;

    ConvBenchmark(const std::string& name)
        : Benchmark(name), conv_identity(this, "conv_identity"),
          conv_identity_longname(this, "conv_identity_longname"),
          conv_linear(this, "conv_linear"),
          conv_linear_longname(this, "conv_linear_longname")
    {
        repetitions = 500;
    }

    void setup_main() { Benchmark::setup_main(); }

    void teardown_main() { Benchmark::teardown_main(); }

    void main() override
    {
        conv_identity.collect([&]() {
            for (unsigned i = 0; i < 1000; ++i)
            {
                convert_units("m", "M", 100.5);
                convert_units("M", "m", 100.5);
                convert_units("sec", "S", 10);
                convert_units("S", "sec", 10);
            }
        });
        conv_identity_longname.collect([&]() {
            for (unsigned i = 0; i < 1000; ++i)
            {
                convert_units("degree true", "DEGREE TRUE", 60);
                convert_units("DEGREE TRUE", "degree true", 60);
                convert_units("m**(2/3)/S", "M**(2/3)/S", 123.4);
                convert_units("M**(2/3)/S", "m**(2/3)/S", 123.4);
            }
        });
        conv_linear.collect([&]() {
            for (unsigned i = 0; i < 1000; ++i)
            {
                convert_units("K", "C", 279.51);
                convert_units("C", "K", 27.5);
                convert_units("M", "FT", 2);
                convert_units("FT", "M", 2);
            }
        });
        conv_linear_longname.collect([&]() {
            for (unsigned i = 0; i < 1000; ++i)
            {
                convert_units("cal/s/cm**2", "W/M**2", 279.51);
                convert_units("Mj/m**2", "J/M**2", 279.51);
                convert_units("cal/cm**2", "J/M**2", 123.4);
                convert_units("J/M**2", "cal/cm**2", 123.4);
            }
        });
    }
} test("conv");

} // namespace
