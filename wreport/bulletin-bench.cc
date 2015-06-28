#include "benchmark.h"
#include "bulletin.h"
#include <vector>
#include <cstdlib>
#include <cassert>

using namespace wreport;
using namespace wreport::benchmark;
using namespace std;

namespace {

template<typename Bltn>
struct TestData
{
    string fname;
    string data;
    Bltn* head_bulletin;
    Bltn* data_bulletin;

    TestData(const std::string& fname, const std::string& data)
        : fname(fname), data(data),
          head_bulletin(Bltn::create().release()),
          data_bulletin(Bltn::create().release())
    {
    }
    TestData(const TestData&) = delete;
    TestData(TestData&& o)
        : fname(move(o.fname)), data(move(o.data)), head_bulletin(o.head_bulletin), data_bulletin(o.data_bulletin)
    {
        o.head_bulletin = o.data_bulletin = nullptr;
    }
    ~TestData()
    {
        delete head_bulletin;
        delete data_bulletin;
    }
    TestData& operator=(const TestData&) = delete;
};

template<typename Bltn>
void load(const char* msgdir, vector<TestData<Bltn>>& out, vector<string> fnames)
{
    const char* datadir = getenv("WREPORT_TESTDATA");
    assert(datadir != nullptr);
    for (const auto& fname: fnames)
    {
        string pathname = datadir;
        pathname += "/";
        pathname += msgdir;
        pathname += "/";
        pathname += fname;
        FILE* in = fopen(pathname.c_str(), "rb");
        assert(in != nullptr);
        string buf;
        long offset = 0;
        unsigned count = 0;
        while (Bltn::read(in, buf, pathname.c_str(), &offset))
        {
            out.emplace_back(fname, buf);
            ++count;
        }
        fclose(in);
        //fprintf(stderr, "%u\t%s\n", count, fname.c_str());
    }
}

struct BulletinBenchmark : Benchmark
{
    vector<TestData<BufrBulletin>> bufr_data;
    vector<TestData<CrexBulletin>> crex_data;
    Task decode_bufr_head;
    Task decode_bufr;
    Task decode_crex_head;
    Task decode_crex;
    Task encode_bufr;
    Task encode_crex;

    BulletinBenchmark(const std::string& name)
        : Benchmark(name),
          decode_bufr_head(this, "decode_bufr_head"), decode_bufr(this, "decode_bufr"),
          decode_crex_head(this, "decode_crex_head"), decode_crex(this, "decode_crex"),
          encode_bufr(this, "encode_bufr"), encode_crex(this, "encode_bufr")
    {

        repetitions = 20;
    }

    void setup_main()
    {
        Benchmark::setup_main();
        load<BufrBulletin>("bufr", bufr_data, { "airep-old-4-142.bufr", "A_ISMN02LFPW080000RRA_C_RJTD_20140808000319_100.bufr", "ascat1.bufr", "atms1.bufr", "atms2.bufr", "bufr1", "bufr2", "bufr3", "C04004.bufr", "C04-B31021-1.bufr", "C04type21.bufr", "C05060.bufr", "C06006.bufr", "C08022.bufr", "C08032-toolong.bufr", "C23000-1.bufr", "C23000.bufr", "ed4-compr-string.bufr", "ed4date.bufr", "ed4-empty.bufr", "ed4-parseerror1.bufr", "gps_zenith.bufr", "gts-buoy1.bufr", "gts-synop-rad1.bufr", "gts-synop-rad2.bufr", "gts-synop-tchange.bufr", "new-003.bufr", "noassoc.bufr", "obs0-1.11188.bufr", "obs0-1.22.bufr", "obs0-3.504.bufr", "obs1-11.16.bufr", "obs1-13.36.bufr", "obs1-140.454.bufr", "obs1-19.3.bufr", "obs1-9.2.bufr", "obs2-101.16.bufr", "obs2-102.1.bufr", "obs2-91.2.bufr", "obs4-142.1.bufr", "obs4-144.4.bufr", "obs4-145.4.bufr", "synop-cloudbelow.bufr", "synop-evapo.bufr", "synop-groundtemp.bufr", "synop-longname.bufr", "synop-oddgust.bufr", "synop-oddprec.bufr", "synop-old-buoy.bufr", "synop-radinfo.bufr", "synop-strayvs.bufr", "synop-sunshine.bufr", "synop-tchange.bufr", "synotemp.bufr", "table17.bufr", "temp-gts1.bufr", "temp-gts2.bufr", "temp-gts3.bufr", "test-airep1.bufr", "test-buoy1.bufr", "test-soil1.bufr", "test-temp1.bufr" });
        load<CrexBulletin>("crex", crex_data, { "test-mare0.crex", "test-mare1.crex", "test-mare2.crex", "test-synop0.crex", "test-synop1.crex", "test-synop2.crex", "test-synop3.crex", "test-temp0.crex" });
    }

    void teardown_main()
    {
        Benchmark::teardown_main();
    }

    void main() override
    {
        decode_bufr_head.collect([&]() {
            for (auto& d: bufr_data)
                d.head_bulletin->decode_header(d.data);
        });
        decode_bufr.collect([&]() {
            for (auto& d: bufr_data)
            {
                //fprintf(stderr, "data %s\n", d.fname.c_str());
                d.data_bulletin->decode(d.data);
            }
        });
        decode_crex_head.collect([&]() {
            for (auto& d: crex_data)
                d.head_bulletin->decode_header(d.data);
        });
        decode_crex.collect([&]() {
            for (auto& d: crex_data)
                d.data_bulletin->decode(d.data);
        });
        encode_bufr.collect([&]() {
            for (auto& d: bufr_data)
            {
                string out;
                d.data_bulletin->encode(out);
            }
        });
        encode_crex.collect([&]() {
            for (auto& d: crex_data)
            {
                string out;
                d.data_bulletin->encode(out);
            }
        });
    }
} test("bulletin");

}


