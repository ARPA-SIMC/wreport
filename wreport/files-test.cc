#include "files.h"
#include "tests.h"
#include <cstdio>

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override;
} test("files");

std::string peek_file(FILE* fd)
{
    long pos = ftell(fd);
    char buf[512];
    wassert(actual(fgets(buf, 511, fd)) != nullptr);
    fseek(fd, pos, SEEK_SET);
    return buf;
}

void Tests::register_tests()
{
    add_method("seek_past_signature", []() {
        FILE* fd = tmpfile();
        wassert(actual(fputs("foo bar baz\n", fd)) > 0);
        wassert(actual(fseek(fd, 0, SEEK_SET)) == 0);

        InputFileStdio in(fd, 0, "test");

        wassert(actual(in.seek_past_signature("foo", 3)) == 0);
        wassert(actual(peek_file(fd)) == " bar baz\n");

        wassert(actual(in.seek_past_signature("baz", 3)) == 8);
        wassert(actual(peek_file(fd)) == "\n");
    });

    add_method("read_until_crex_terminator", []() {
        FILE* fd = tmpfile();
        wassert(actual(fputs("CREX++ this is a test ++\n7777\n", fd)) > 0);
        wassert(actual(fseek(fd, 4, SEEK_SET)) == 0);

        InputFileStdio in(fd, 0, "test");
        std::string buf;
        in.read_until_crex_terminator(buf);
        wassert(actual(buf) == "++ this is a test ++\n7777");
        wassert(actual(peek_file(fd)) == "\n");
    });
}

} // namespace
