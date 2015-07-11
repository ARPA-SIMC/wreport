#include "bulletin.h"
#include "bulletin/internals.h"
#include "buffers/crex.h"
#include "config.h"

// #define TRACE_ENCODER

#ifdef TRACE_ENCODER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

namespace {

struct DDSEncoder : public bulletin::UncompressedEncoder
{
    buffers::CrexOutput& ob;

    DDSEncoder(const Bulletin& b, unsigned subset_no, buffers::CrexOutput& ob)
        : UncompressedEncoder(b, subset_no), ob(ob)
    {
        TRACE("start_subset %u\n", subset_no);

        /* Encode the subsection terminator */
        if (subset_no > 0)
            ob.raw_append("+\r\r\n", 4);
    }
    virtual ~DDSEncoder() {}

    void define_variable(Varinfo info) override
    {
        const Var& var = get_var();
        IFTRACE {
            TRACE("encode_var ");
            var.print(stderr);
        }
        ob.append_var(info, var);
    }

    unsigned define_delayed_replication_factor(Varinfo info) override
    {
        const Var& var = get_var();
        IFTRACE {
            TRACE("encode_semantic_var ");
            var.print(stderr);
        }
        switch (info->code)
        {
            case WR_VAR(0, 31, 1):
            case WR_VAR(0, 31, 2):
            case WR_VAR(0, 31, 11):
            case WR_VAR(0, 31, 12):
            {
                unsigned count = var.enqi();

                /* Encode the repetition count */
                ob.raw_append(" ", 1);
                ob.encode_check_digit();
                ob.raw_appendf("%04u", count);

                return count;
            }
            default:
                ob.append_var(info, var);
                if (var.isset())
                    return var.enqi();
                else
                    return 0xffffffff;
        }
    }
};

void encode_sec1(const CrexBulletin& in, buffers::CrexOutput out)
{
    if (in.data_subcategory == 0xff)
        out.raw_appendf("T%02hhd%02hhd%02hhd A%03hhd",
                in.master_table_number,
                in.edition_number,
                in.master_table_version_number,
                in.data_category);
    else
        out.raw_appendf("T%02hhd%02hhd%02hhd A%03hhd%03hhd",
                in.master_table_number,
                in.edition_number,
                in.master_table_version_number,
                in.data_category,
                in.data_subcategory);

    /* Encode the data descriptor section */

    for (vector<Varcode>::const_iterator i = in.datadesc.begin();
            i != in.datadesc.end(); ++i)
    {
        char prefix;
        switch (WR_VAR_F(*i))
        {
            case 0: prefix = 'B'; break;
            case 1: prefix = 'R'; break;
            case 2: prefix = 'C'; break;
            case 3: prefix = 'D'; break;
            default: prefix = '?'; break;
        }

        // Don't put delayed replication counts in the data section
        if (WR_VAR_F(*i) == 0 && WR_VAR_X(*i) == 31 && WR_VAR_Y(*i) < 3)
            continue;

        out.raw_appendf(" %c%02d%03d", prefix, WR_VAR_X(*i), WR_VAR_Y(*i));
    }

    if (out.has_check_digit)
    {
        out.raw_append(" E", 2);
        out.expected_check_digit = 1;
    }

    out.raw_append("++\r\r\n", 5);
}

}

void CrexBulletin::encode(std::string& buf) const
{
    buffers::CrexOutput out(buf);

    // Encode section 0
    out.raw_append("CREX++\r\r\n", 9);

    // Encode section 1
    int sec1_start = out.buf.size();
    encode_sec1(*this, out);
    TRACE("SEC1 encoded as [[[%s]]]", out.buf.substr(sec1_start).c_str());

    /* Encode section 2 */
    int sec2_start = out.buf.size();

    // Encode all subsets
    for (unsigned i = 0; i < subsets.size(); ++i)
    {
        DDSEncoder e(*this, i, out);
        e.run();
    }
    out.raw_append("++\r\r\n", 5);

    TRACE("SEC2 encoded as [[[%s]]]", out.buf.substr(sec2_start).c_str());

    /* Encode section 3 */
    int sec3_start = out.buf.size();
    /* Nothing to do, as we have no custom section */

    /* Encode section 4 */
    int sec4_start = out.buf.size();
    out.raw_append("7777\r\r\n", 7);
}

}
