#include "benchmark.h"
#include "var.h"
#include <vector>
#include <cstdlib>

using namespace wreport;
using namespace wreport::benchmark;
using namespace std;

namespace {

struct VarBenchmark : Benchmark
{
    _Varinfo varinfo_int;
    _Varinfo varinfo_double;
    _Varinfo varinfo_string;
    _Varinfo varinfo_binary;
    static const unsigned vars_count = 30000;
    Var* vars_unset;
    Var* vars_i;
    Var* vars_d;
    Var* vars_c;
    Var* vars_b;
    Task create_unset;
    Task create_i;
    Task create_d;
    Task create_c;
    Task create_b;
    Task isset;
    Task enqi;
    Task enqd;
    Task enqc;
    Task enqb;
    Task unset;
    Task seti;
    Task setd;
    Task setc;
    Task setb;

    VarBenchmark(const std::string& name)
        : Benchmark(name),
          create_unset(this, "new"),
          create_i(this, "newi"),
          create_d(this, "newd"),
          create_c(this, "newc"),
          create_b(this, "newb"),
          isset(this, "isset"),
          enqi(this, "enqi"),
          enqd(this, "enqd"),
          enqc(this, "enqc"),
          enqb(this, "enqb"),
          unset(this, "unset"),
          seti(this, "seti"),
          setd(this, "setd"),
          setc(this, "setc"),
          setb(this, "setb")
    {
        repetitions = 100;
    }

    void setup_main()
    {
        Benchmark::setup_main();
        varinfo_int.set_bufr(WR_VAR(0, 0, 0), "test integer variable", "number", 0, 10, 0, 10);
        varinfo_double.set_bufr(WR_VAR(0, 0, 0), "test double variable", "number", 5, 10, -100000, 10);
        varinfo_string.set_string(WR_VAR(0, 0, 0), "test string variable", 32);
        varinfo_binary.set_binary(WR_VAR(0, 0, 0), "test binary variable", 20);
        // Allocate space for the test vars
        vars_unset = (Var*)malloc(vars_count * sizeof(Var));
        vars_i = (Var*)malloc(vars_count * sizeof(Var));
        vars_d = (Var*)malloc(vars_count * sizeof(Var));
        vars_c = (Var*)malloc(vars_count * sizeof(Var));
        vars_b = (Var*)malloc(vars_count * sizeof(Var));
    }

    void teardown_main()
    {
        Benchmark::teardown_main();
        free(vars_unset);
        free(vars_i);
        free(vars_d);
        free(vars_c);
        free(vars_b);
    }

    void main() override
    {
        create_unset.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                switch (i % 4)
                {
                    case 0: new(&vars_unset[i]) Var(&varinfo_int); break;
                    case 1: new(&vars_unset[i]) Var(&varinfo_double); break;
                    case 2: new(&vars_unset[i]) Var(&varinfo_string); break;
                    case 3: new(&vars_unset[i]) Var(&varinfo_binary); break;
                }
            }
        });
        create_i.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                switch (i % 4)
                {
                    case 0: new(&vars_i[i]) Var(&varinfo_int,           0); break;
                    case 1: new(&vars_i[i]) Var(&varinfo_int,         100); break;
                    case 2: new(&vars_i[i]) Var(&varinfo_int,  1234567890); break;
                    case 3: new(&vars_i[i]) Var(&varinfo_int, -1234567890); break;
                }
            }
        });
        create_d.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                switch (i % 4)
                {
                    case 0: new(&vars_d[i]) Var(&varinfo_double,     0.0); break;
                    case 1: new(&vars_d[i]) Var(&varinfo_double,    -1.0); break;
                    case 2: new(&vars_d[i]) Var(&varinfo_double,  1234.56789); break;
                    case 3: new(&vars_d[i]) Var(&varinfo_double, -1234.56789); break;
                }
            }
        });
        create_c.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                switch (i % 4)
                {
                    case 0: new(&vars_c[i]) Var(&varinfo_string, ""); break;
                    case 1: new(&vars_c[i]) Var(&varinfo_string, "foo"); break;
                    case 2: new(&vars_c[i]) Var(&varinfo_string, "lorem ipsum dolor sit antani"); break;
                    case 3: new(&vars_c[i]) Var(&varinfo_string, "foobarbaz"); break;
                }
            }
        });
        create_b.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                switch (i % 4)
                {
                    case 0: new(&vars_b[i]) Var(&varinfo_binary, "\x00\x00"); break;
                    case 1: new(&vars_b[i]) Var(&varinfo_binary, "\xff\xff"); break;
                    case 2: new(&vars_b[i]) Var(&varinfo_binary, "\xaa\xaa"); break;
                    case 3: new(&vars_b[i]) Var(&varinfo_binary, "\xca\xfe"); break;
                }
            }
        });
        // Query the variables
        isset.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_unset[i].isset();
                vars_i[i].isset();
                vars_d[i].isset();
                vars_c[i].isset();
                vars_b[i].isset();
            }
        });
        enqi.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].enqi();
                vars_i[i].enqi();
                vars_i[i].enqi();
                vars_i[i].enqi();
                vars_d[i].enqi();
            }
        });
        enqd.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].enqd();
                vars_d[i].enqd();
                vars_d[i].enqd();
                vars_d[i].enqd();
                vars_d[i].enqd();
            }
        });
        enqc.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].enqc();
                vars_d[i].enqc();
                vars_c[i].enqc();
                vars_c[i].enqc();
                vars_b[i].enqc();
            }
        });
        enqb.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_b[i].enqc();
                vars_b[i].enqc();
                vars_b[i].enqc();
                vars_b[i].enqc();
                vars_b[i].enqc();
            }
        });
        // Set the variables to something
        unset.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_unset[i].unset();
                vars_i[i].unset();
                vars_d[i].unset();
                vars_c[i].unset();
                vars_b[i].unset();
            }
        });
        seti.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].seti(          0);
                vars_i[i].seti(        100);
                vars_i[i].seti( 1234567890);
                vars_i[i].seti(-1234567890);
                vars_d[i].seti(-1234567890);
            }
        });
        setd.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].setd( 1234567890);
                vars_d[i].setd(    0.0    );
                vars_d[i].setd(   -1.0    );
                vars_d[i].setd( 1234.56789);
                vars_d[i].setd(-1234.56789);
            }
        });
        setc.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_i[i].setc("123567890");
                vars_d[i].setc("123567890");
                vars_c[i].setc("");
                vars_c[i].setc("lorem ipsum dolor sit antani");
                vars_b[i].setc("foo");
            }
        });
        setb.collect([&]() {
            for (unsigned i = 0; i < vars_count; ++i)
            {
                vars_b[i].setc("\x00\x00");
                vars_b[i].setc("\xff\xff");
                vars_b[i].setc("\xaa\xaa");
                vars_b[i].setc("\xca\xfe");
                vars_b[i].setc("\xf0\xf0");
            }
        });
    }
} test("var");

}

