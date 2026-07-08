#include "tests.h"
#include "utils/json.h"
#include "utils/string.h"
#include "utils/sys.h"
#include <cstdlib>
#include <cstring>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace wreport {
namespace tests {

std::filesystem::path datafile(const std::filesystem::path& fname)
{
    const char* testdatadirenv = getenv("WREPORT_TESTDATA");
    std::filesystem::path testdatadir(testdatadirenv ? testdatadirenv : ".");
    return testdatadir / fname;
}

std::filesystem::path path_from_env(const char* varname, const char* deflt)
{
    if (const char* val = getenv(varname))
        return std::filesystem::path(val);
    else if (deflt)
        return deflt;
    else
        return std::filesystem::current_path();
}

std::string slurpfile(const std::filesystem::path& name)
{
    return sys::read_file(datafile(name));
}

std::vector<std::filesystem::path> all_test_files(const std::string& encoding)
{
    const char* testdatadirenv = getenv("WREPORT_TESTDATA");
    std::filesystem::path testdatadir(testdatadirenv ? testdatadirenv : ".");
    testdatadir /= encoding;

    std::vector<std::filesystem::path> res;
    std::filesystem::path relroot(encoding);
    sys::Path dir(testdatadir);
    for (const auto& i : dir)
        if (str::endswith(i.d_name, "." + encoding))
            res.push_back(relroot / i.d_name);
    return res;
}

namespace {
void add_header(json::Dict& d, const Bulletin& bulletin)
{
    d.add_unsigned("mtn", (int)bulletin.master_table_number);
    d.add_nullable("dc", bulletin.data_category, 0xff);
    d.add_nullable("ds", bulletin.data_subcategory, 0xff);
    d.add_nullable("dsl", bulletin.data_subcategory_local, 0xff);
    d.add_nullable("oc", bulletin.originating_centre, 0xffff);
    d.add_nullable("osc", bulletin.originating_subcentre, 0xffff);
    d.add_unsigned("usn", bulletin.update_sequence_number);

    if (const auto* bufr = dynamic_cast<const BufrBulletin*>(&bulletin))
    {
        d.add_unsigned("ed", bufr->edition_number);
        d.add_nullable("mtvn", bufr->master_table_version_number, 0xff);
        d.add_nullable("mtvn", bufr->master_table_version_number, 0xff);
        d.add_bool("c", bufr->compression);
    }
    else if (const auto& crex = dynamic_cast<const CrexBulletin*>(&bulletin))
    {
        d.add_unsigned("ed", crex->edition_number);
        d.add_nullable("mtvn", crex->master_table_version_number, 0xff);
        d.add_nullable("mtvnb", crex->master_table_version_number_bufr, 0xff);
        d.add_nullable("mtvnl", crex->master_table_version_number_local, 0xff);
        d.add_bool("c", crex->has_check_digit);
    }
    else
    {
        error_consistency::throwf("unsupported bulletin type");
    }

    d.add_nullable("ye", bulletin.rep_year, 0xffff);
    d.add_nullable("mo", bulletin.rep_month, 0xff);
    d.add_nullable("da", bulletin.rep_day, 0xff);
    d.add_nullable("ho", bulletin.rep_hour, 0xff);
    d.add_nullable("mi", bulletin.rep_minute, 0xff);
    d.add_nullable("se", bulletin.rep_second, 0xff);
}

void add_var(json::Dict& d, const Var& var)
{
    d.add("c", varcode_format(var.code()));
    if (var.isset())
        d.add("v", var.enqs());
    if (var.next_attr())
    {
        auto attrs = d.add_list("a");
        for (const Var* a = var.next_attr(); a != NULL; a = a->next_attr())
        {
            auto attr = attrs.add_dict();
            add_var(attr, *a);
        }
    }
}

} // namespace

void dump_jsonl(const Bulletin& bulletin, std::stringstream& buf)
{
    using namespace wreport::json;

    JSONL out(buf);
    auto bdict = out.dict();
    {
        auto head = bdict.add_dict("h");
        add_header(head, bulletin);
    }
    {
        auto subsets = bdict.add_list("s");
        for (const auto& subset : bulletin.subsets)
        {
            auto vars = subsets.add_list();
            for (const auto& var : subset)
            {
                auto v = vars.add_dict();
                add_var(v, var);
            }
        }
    }
}

void track_bulletin(Bulletin& b, const char* tag,
                    const std::filesystem::path& fname)
{
    std::filesystem::path dumpfname =
        "/tmp/bulletin-"s + fname.filename().native() + "-" + tag;
    FILE* out = fopen(dumpfname.c_str(), "wt");
    fprintf(out, "Contents of %s %s:\n", fname.c_str(), tag);
    b.print(out);
    fprintf(out, "\nData descriptor section of %s %s:\n", fname.c_str(), tag);
    b.print_datadesc(out);
    fprintf(out, "\nStructure of %s %s:\n", fname.c_str(), tag);
    b.print_structured(out);
    fclose(out);
    fprintf(stderr, "%s %s dumped as %s\n", fname.c_str(), tag,
            dumpfname.c_str());
}

namespace {
void compare_values(const Var& avar, const Var& evar, const std::string& name)
{
    if (!avar.value_equals(evar))
    {
        std::stringstream ss;
        ss << name << " values differ: ";
        if (!evar.isset())
            ss << "expected undefined";
        else
            ss << "expected is " << evar.format();
        ss << " but actual ";
        if (!avar.isset())
            ss << "is undefined";
        else
            ss << "is " << avar.format();
        throw TestFailed(ss.str());
    }
}
} // namespace

void assert_var_equal(const Var& avar, const Var& evar)
{
    // Code
    if (avar.code() != evar.code())
    {
        std::stringstream ss;
        ss << "variable codes differ: expected " << varcode_format(evar.code())
           << " actual " << varcode_format(avar.code());
        throw TestFailed(ss.str());
    }

    // Value
    compare_values(avar, evar, "variable");

    // Attributes
    const Var* aattr = avar.next_attr();
    const Var* eattr = evar.next_attr();
    while (true)
    {
        if (!aattr && !eattr)
            break;
        // If both exists but codes are different, one of the two is missing an
        // attribute
        if (aattr && eattr && aattr->code() != eattr->code())
        {
            // Set the highest one to NULL, and use the next check to
            // trigger the appropriate test failure
            if (aattr->code() < eattr->code())
                eattr = NULL;
            else
                aattr = NULL;
        }
        if (!aattr || !eattr)
        {
            std::stringstream ss;
            ss << "attributes differ: ";
            if (aattr)
                ss << "actual has " << varcode_format(aattr->code())
                   << " which was not expected";
            else
                ss << "actual does not have attribute "
                   << varcode_format(eattr->code())
                   << " which was expected to be "
                   << eattr->format("undefined");
            throw TestFailed(ss.str());
        }

        compare_values(*aattr, *eattr,
                       "attribute " + varcode_format(aattr->code()));

        // Move to the next attribute
        aattr = aattr->next_attr();
        eattr = eattr->next_attr();
    }
}

void assert_var_not_equal(const Var& actual, const Var& expected)
{
    if (actual == expected)
    {
        std::stringstream ss;
        ss << "variables should differ, but are the same";
        throw TestFailed(ss.str());
    }
}

namespace {
template <typename T> bool equals(T a, T b) { return a == b; }
bool equals(const char* a, const char* b) { return strcmp(a, b) == 0; }
} // namespace

template <typename Val>
void assert_var_value_equal(const Var& actual, Val expected)
{
    Var vexpected(actual.info(), expected);
    if (!actual.value_equals(vexpected))
    {
        std::stringstream ss;
        ss << "actual variable value is " << actual.format() << " ("
           << actual.enq<Val>() << ") instead of " << expected;
        throw TestFailed(ss.str());
    }
}

template <typename Val>
void assert_var_value_not_equal(const Var& actual, Val expected)
{
    if (equals(actual.enq<Val>(), expected))
    {
        std::stringstream ss;
        ss << "actual variable value is " << actual.format()
           << " while it should be " << expected;
        throw TestFailed(ss.str());
    }
}

template void assert_var_value_equal<int>(const Var& actual, int expected);
template void assert_var_value_equal<double>(const Var& actual,
                                             double expected);
template void assert_var_value_equal<char*>(const Var& actual, char* expected);
template void assert_var_value_equal<char const*>(const Var& actual,
                                                  char const* expected);
template void assert_var_value_equal<std::string>(const Var& actual,
                                                  std::string expected);
template void assert_var_value_not_equal<int>(const Var& actual, int expected);
template void assert_var_value_not_equal<double>(const Var& actual,
                                                 double expected);
template void assert_var_value_not_equal<char*>(const Var& actual,
                                                char* expected);
template void assert_var_value_not_equal<char const*>(const Var& actual,
                                                      char const* expected);
template void assert_var_value_not_equal<std::string>(const Var& actual,
                                                      std::string expected);

void ActualVar::isset() const
{
    if (_actual.isset())
        return;
    std::stringstream ss;
    ss << "actual variable is unset, but it should not be";
    throw TestFailed(ss.str());
}

void ActualVar::isunset() const
{
    if (!_actual.isset())
        return;
    std::stringstream ss;
    ss << "actual variable value is " << _actual.format()
       << ", but it should be unset";
    throw TestFailed(ss.str());
}

void ActualVarcode::operator==(Varcode expected) const
{
    if (expected == _actual)
        return;
    std::stringstream ss;
    ss << "actual varcode value is " << varcode_format(_actual)
       << " but it should be " << varcode_format(expected);
    throw TestFailed(ss.str());
}

void ActualVarcode::operator!=(Varcode expected) const
{
    if (expected != _actual)
        return;
    std::stringstream ss;
    ss << "actual varcode value is " << varcode_format(_actual)
       << " but it should not be";
    throw TestFailed(ss.str());
}

template <typename BULLETIN> void TestCodec<BULLETIN>::run()
{
    WREPORT_TEST_INFO(test_info);

    // Read the whole contents of the test file
    std::string raw1 = wcallchecked(slurpfile(fname));

    test_info() << fname << ": decode original version";
    auto msg1 = wcallchecked(decode_checked<BULLETIN>(raw1, fname.c_str()));
    wassert(check_contents(*msg1));

    test_info() << fname << ": decode original version, verbose version";
    FILE* out = fopen("/dev/null", "w");
    auto msg1a =
        wcallchecked(decode_checked<BULLETIN>(raw1, fname.c_str(), out));
    wassert(check_contents(*msg1a));
    fclose(out);

    test_info() << fname << ": comparing normal and verbose decoder output";
    {
        notes::Collect c(std::cerr);
        unsigned diffs = msg1->diff(*msg1a);
        if (diffs)
        {
            track_bulletin(*msg1, "orig", fname.c_str());
            track_bulletin(*msg1a, "verb", fname.c_str());
        }
        wassert(actual(diffs) == 0u);
    }

    // Encode it again
    test_info() << fname << ": re-encode original version";
    std::string raw = wcallchecked(msg1->encode());

    // Decode our encoder's output
    test_info() << fname << ": decode what we encoded";
    auto msg2 = wcallchecked(decode_checked<BULLETIN>(raw, fname.c_str()));

    // Test the decoded version
    wassert(check_contents(*msg2));

    // Ensure the two are the same
    test_info() << fname << ": comparing original and re-encoded";
    notes::Collect c(std::cerr);
    unsigned diffs = msg1->diff(*msg2);
    if (diffs)
    {
        track_bulletin(*msg1, "orig", fname.c_str());
        track_bulletin(*msg2, "reenc", fname.c_str());
    }
    wassert(actual(diffs) == 0u);
}

template class TestCodec<BufrBulletin>;
template class TestCodec<CrexBulletin>;

} // namespace tests
} // namespace wreport
