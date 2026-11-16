// SPDX-License-Identifier: GPL-2.0-only

#include "files.h"
#include "tests.h"
#include <cassert>
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

class Tempfile
{
    FILE* fd;

public:
    explicit Tempfile(const std::string& contents) : fd(tmpfile())
    {
        int res = fputs(contents.c_str(), fd);
        assert(res > 0);
        res = fseek(fd, 0, SEEK_SET);
        assert(res == 0);
    }
    ~Tempfile() { fclose(fd); }

    operator FILE*() { return fd; }

    std::string peek()
    {
        long pos = ftell(fd);
        char buf[512];
        wassert(actual(fgets(buf, 511, fd)) != nullptr);
        fseek(fd, pos, SEEK_SET);
        return buf;
    }
};

class Popen
{
    FILE* fd;

public:
    explicit Popen(const std::string& command)
    {
        fd = popen(command.c_str(), "r");
        assert(fd != nullptr);
    }
    ~Popen() { pclose(fd); }

    operator FILE*() { return fd; }

    std::string read()
    {
        char buf[512];
        wassert(actual(fgets(buf, 511, fd)) != nullptr);
        return buf;
    }
};

void Tests::register_tests()
{
    add_method("seek_past_signature", []() {
        Tempfile fd("foo bar baz\n");

        InputFileStdio in(fd, 0, "test");

        wassert(actual(in.seek_past_signature("foo", 3)) == 0);
        wassert(actual(fd.peek()) == " bar baz\n");

        wassert(actual(in.seek_past_signature("baz", 3)) == 8);
        wassert(actual(fd.peek()) == "\n");
    });

    add_method("read_until_crex_terminator", []() {
        Tempfile fd("CREX++ this is a test ++\n7777\n");
        wassert(actual(fseek(fd, 4, SEEK_SET)) == 0);

        InputFileStdio in(fd, 0, "test");
        std::string buf;
        in.read_until_crex_terminator(buf);
        wassert(actual(buf) == "++ this is a test ++\n7777");
        wassert(actual(fd.peek()) == "\n");
    });

    add_method("seek_past_signature_pipe_start", []() {
        Popen fd("echo foo bar baz");
        InputFileStdio in(fd, 0, "test");

        wassert(actual(in.seek_past_signature("foo", 3)) == 0);
        wassert(actual(fd.read()) == " bar baz\n");
    });

    add_method("seek_past_signature_pipe_forward", []() {
        Popen fd("echo foo bar baz");
        InputFileStdio in(fd, 0, "test");

        wassert(actual(in.seek_past_signature("baz", 3)) == 8);
        wassert(actual(fd.read()) == "\n");
    });

    add_method("read_until_crex_terminator_pipe", []() {
        Popen fd("echo '++ this is a test ++\n7777'");
        InputFileStdio in(fd, 0, "test");
        std::string buf;
        in.read_until_crex_terminator(buf);
        wassert(actual(buf) == "++ this is a test ++\n7777");
        wassert(actual(fd.read()) == "\n");
    });
}

} // namespace
