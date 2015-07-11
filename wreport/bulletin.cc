#include "bulletin.h"
#include "error.h"
#include "tableinfo.h"
#include "vartable.h"
#include "dtable.h"
#include "bulletin/dds-printer.h"
#include "notes.h"
#include <netinet/in.h>
#include "config.h"

using namespace std;

namespace wreport {

namespace {

bool seek_past_signature(FILE* fd, const char* sig, unsigned sig_len, const char* fname)
{
    unsigned got = 0;
    int c;

    errno = 0;

    while (got < sig_len && (c = getc(fd)) != EOF)
    {
        if (c == sig[got])
            got++;
        else
            got = 0;
    }

    if (errno != 0)
    {
        if (fname)
            error_system::throwf("looking for start of %.4s data in %s:", sig, fname);
        else
            error_system::throwf("looking for start of %.4s data", sig);
    }

    if (got != sig_len)
    {
        /* End of file: return accordingly */
        return false;
    }
    return true;
}

}


/*
 * Bulletin
 */

Bulletin::Bulletin() {}
Bulletin::~Bulletin() {}

void Bulletin::clear()
{
    fname.clear();
    offset = 0;
    data_category = data_subcategory = data_subcategory_local = 0xff;
    originating_centre = originating_subcentre = 0xffff;
    update_sequence_number = 0;
    rep_year = 0;
    rep_month = rep_day = rep_hour = rep_minute = rep_second = 0;
    tables.clear();
    datadesc.clear();
    subsets.clear();
}

Subset& Bulletin::obtain_subset(unsigned subsection)
{
    while (subsection >= subsets.size())
        subsets.emplace_back(tables);
    return subsets[subsection];
}

const Subset& Bulletin::subset(unsigned subsection) const
{
	if (subsection >= subsets.size())
		error_notfound::throwf("Requested subset %u but there are only %zd available",
				subsection, subsets.size());
	return subsets[subsection];
}

void Bulletin::print(FILE* out) const
{
    fprintf(out, "%s %hhu:%hhu:%hhu %hu:%hu %04hu-%02hu-%02hu %02hu:%02hu:%02hu %hhu %zd subsets\n",
        encoding_name(),
        data_category, data_subcategory, data_subcategory_local,
        originating_centre, originating_subcentre,
        rep_year, rep_month, rep_day, rep_hour, rep_minute, rep_second,
        update_sequence_number,
        subsets.size());
    fprintf(out, " Tables: %s %s\n",
        tables.btable ? tables.btable->pathname().c_str() : "(not loaded)",
        tables.dtable ? tables.dtable->pathname().c_str() : "(not loaded)");
    fprintf(out, " Data descriptors:\n");
    for (vector<Varcode>::const_iterator i = datadesc.begin(); i != datadesc.end(); ++i)
        fprintf(out, "  %d%02d%03d\n", WR_VAR_F(*i), WR_VAR_X(*i), WR_VAR_Y(*i));
    print_details(out);
    fprintf(out, " Variables:\n");
    for (unsigned i = 0; i < subsets.size(); ++i)
    {
        const Subset& s = subset(i);
        for (unsigned j = 0; j < s.size(); ++j)
        {
            fprintf(out, "  [%d][%d] ", i, j);
            s[j].print(out);
        }
    }
}

void Bulletin::print_structured(FILE* out) const
{
    fprintf(out, "%s %hhu:%hhu:%hhu %hu:%hu %04hu-%02hu-%02hu %02hu:%02hu:%02hu %hhu %zd subsets\n",
        encoding_name(),
        data_category, data_subcategory, data_subcategory_local,
        originating_centre, originating_subcentre,
        rep_year, rep_month, rep_day, rep_hour, rep_minute, rep_second,
        update_sequence_number,
        subsets.size());
    fprintf(out, " Tables: %s %s\n",
            tables.btable ? tables.btable->pathname().c_str() : "(not loaded)",
            tables.dtable ? tables.dtable->pathname().c_str() : "(not loaded)");
    fprintf(out, " Data descriptors:\n");
    for (vector<Varcode>::const_iterator i = datadesc.begin(); i != datadesc.end(); ++i)
        fprintf(out, "  %d%02d%03d\n", WR_VAR_F(*i), WR_VAR_X(*i), WR_VAR_Y(*i));
    print_details(out);
    fprintf(out, " Variables:\n");
    // Print all the subsets
    for (unsigned i = 0; i < subsets.size(); ++i)
    {
        bulletin::DDSPrinter printer(*this, out, i);
        printer.run();
    }
}

void Bulletin::print_details(FILE* out) const {}

void Bulletin::print_datadesc(FILE* out, unsigned indent) const
{
    bulletin::Printer printer(tables, datadesc);
    printer.out = out;
    printer.indent = indent;
    printer.run();
}

unsigned Bulletin::diff(const Bulletin& msg) const
{
    unsigned diffs = 0;
    if (string(encoding_name()) != string(msg.encoding_name()))
    {
        notes::logf("Encodings differ (first is %s, second is %s)\n",
                encoding_name(), msg.encoding_name());
        ++diffs;
    } else
        diffs += diff_details(msg);
    if (data_category != msg.data_category)
    {
        notes::logf("Data categories differ (first is %hhu, second is %hhu)\n",
                data_category, msg.data_category);
        ++diffs;
    }
    if (data_subcategory != msg.data_subcategory)
    {
        notes::logf("Data subcategories differ (first is %hhu, second is %hhu)\n",
                data_subcategory, msg.data_subcategory);
        ++diffs;
    }
    if (data_subcategory_local != msg.data_subcategory_local)
    {
        notes::logf("Data local subcategories differ (first is %hhu, second is %hhu)\n",
                data_subcategory_local, msg.data_subcategory_local);
        ++diffs;
    }
    if (originating_centre != msg.originating_centre)
    {
        notes::logf("Originating centres differ (first is %hu, second is %hu)\n",
                originating_centre, msg.originating_centre);
        ++diffs;
    }
    if (originating_subcentre != msg.originating_subcentre)
    {
        notes::logf("Originating subcentres differ (first is %hu, second is %hu)\n",
                originating_subcentre, msg.originating_subcentre);
        ++diffs;
    }
    if (update_sequence_number != msg.update_sequence_number)
    {
        notes::logf("Update sequence numbers differ (first is %hhu, second is %hhu)\n",
                update_sequence_number, msg.update_sequence_number);
        ++diffs;
    }
    if (rep_year != msg.rep_year)
    {
        notes::logf("Reference years differ (first is %hu, second is %hu)\n",
                rep_year, msg.rep_year);
        ++diffs;
    }
    if (rep_month != msg.rep_month)
    {
        notes::logf("Reference months differ (first is %hhu, second is %hhu)\n",
                rep_month, msg.rep_month);
        ++diffs;
    }
    if (rep_day != msg.rep_day)
    {
        notes::logf("Reference days differ (first is %hhu, second is %hhu)\n",
                rep_day, msg.rep_day);
        ++diffs;
    }
    if (rep_hour != msg.rep_hour)
    {
        notes::logf("Reference hours differ (first is %hhu, second is %hhu)\n",
                rep_hour, msg.rep_hour);
        ++diffs;
    }
    if (rep_minute != msg.rep_minute)
    {
        notes::logf("Reference minutes differ (first is %hhu, second is %hhu)\n",
                rep_minute, msg.rep_minute);
        ++diffs;
    }
    if (rep_second != msg.rep_second)
    {
        notes::logf("Reference seconds differ (first is %hhu, second is %hhu)\n",
                rep_second, msg.rep_second);
        ++diffs;
    }

    if (tables.btable == NULL && msg.tables.btable != NULL)
    {
        notes::logf("First message did not load B btables, second message has %s\n",
                msg.tables.btable->pathname().c_str());
        ++diffs;
    } else if (tables.btable != NULL && msg.tables.btable == NULL) {
        notes::logf("Second message did not load B btables, first message has %s\n",
                tables.btable->pathname().c_str());
        ++diffs;
    } else if (tables.btable != NULL && msg.tables.btable != NULL && tables.btable->pathname() != msg.tables.btable->pathname()) {
        notes::logf("B tables differ (first has %s, second has %s)\n",
                tables.btable->pathname().c_str(), msg.tables.btable->pathname().c_str());
        ++diffs;
    }

    if (tables.dtable == NULL && msg.tables.dtable != NULL)
    {
        notes::logf("First message did not load B dtable, second message has %s\n",
                msg.tables.dtable->pathname().c_str());
        ++diffs;
    } else if (tables.dtable != NULL && msg.tables.dtable == NULL) {
        notes::logf("Second message did not load B dtable, first message has %s\n",
                tables.dtable->pathname().c_str());
        ++diffs;
    } else if (tables.dtable != NULL && msg.tables.dtable != NULL && tables.dtable->pathname() != msg.tables.dtable->pathname()) {
        notes::logf("D tables differ (first has %s, second has %s)\n",
                tables.dtable->pathname().c_str(), msg.tables.dtable->pathname().c_str());
        ++diffs;
    }

    if (datadesc.size() != msg.datadesc.size())
    {
        notes::logf("Data descriptor sections differ (first has %zd elements, second has %zd)\n",
                datadesc.size(), msg.datadesc.size());
        ++diffs;
    } else {
        for (unsigned i = 0; i < datadesc.size(); ++i)
            if (datadesc[i] != msg.datadesc[i])
            {
                notes::logf("Data descriptors differ at element %u (first has %01d%02d%03d, second has %01d%02d%03d)\n",
                        i, WR_VAR_F(datadesc[i]), WR_VAR_X(datadesc[i]), WR_VAR_Y(datadesc[i]),
                        WR_VAR_F(msg.datadesc[i]), WR_VAR_X(msg.datadesc[i]), WR_VAR_Y(msg.datadesc[i]));
                ++diffs;
            }
    }

    if (subsets.size() != msg.subsets.size())
    {
        notes::logf("Number of subsets differ (first is %zd, second is %zd)\n",
                subsets.size(), msg.subsets.size());
        ++diffs;
    } else
        for (unsigned i = 0; i < subsets.size(); ++i)
            diffs += subsets[i].diff(msg.subsets[i]);
    return diffs;
}

unsigned Bulletin::diff_details(const Bulletin& msg) const { return 0; }


/*
 * BufrCodecOptions
 */

BufrCodecOptions::BufrCodecOptions() {}

std::unique_ptr<BufrCodecOptions> BufrCodecOptions::create()
{
    return unique_ptr<BufrCodecOptions>(new BufrCodecOptions);
}


/*
 * BufrBulletin
 */

BufrBulletin::BufrBulletin()
{
}

std::unique_ptr<BufrBulletin> BufrBulletin::create()
{
    return unique_ptr<BufrBulletin>(new BufrBulletin);
}

BufrBulletin::~BufrBulletin()
{
}

void BufrBulletin::clear()
{
    Bulletin::clear();
    edition_number = 4;
    master_table_number = 0;
    master_table_version_number = 19;
    master_table_version_number_local = 0;
    compression = false;
    optional_section.clear();
}

void BufrBulletin::load_tables()
{
    tables.load_bufr(BufrTableID(originating_centre, originating_subcentre, master_table_number, master_table_version_number, master_table_version_number_local));
}

void BufrBulletin::print_details(FILE* out) const
{
    fprintf(out, " BUFR details: ed%hhu t%hhu:%hhu:%hhu %c osl%zd\n",
            edition_number,
            master_table_number, master_table_version_number, master_table_version_number_local,
            compression ? 'c' : '-', optional_section.size());
}

unsigned BufrBulletin::diff_details(const Bulletin& bulletin) const
{
    unsigned diffs = Bulletin::diff_details(bulletin);
    const BufrBulletin* bb = dynamic_cast<const BufrBulletin*>(&bulletin);
    if (!bb) throw error_consistency("BufrBulletin::diff_details called with a non-BufrBulletin argument");
    const BufrBulletin& msg = *bb;

    if (edition_number != msg.edition_number)
    {
        notes::logf("BUFR edition numbers differ (first is %hhu, second is %hhu)\n",
                edition_number, msg.edition_number);
        ++diffs;
    }
    if (master_table_number != msg.master_table_number)
    {
        notes::logf("BUFR master table numbers differ (first is %hhu, second is %hhu)\n",
                master_table_number, msg.master_table_number);
        ++diffs;
    }
    if (master_table_version_number != msg.master_table_version_number)
    {
        notes::logf("BUFR master table version numbers differ (first is %hhu, second is %hhu)\n",
                master_table_version_number, msg.master_table_version_number);
        ++diffs;
    }
    if (master_table_version_number_local != msg.master_table_version_number_local)
    {
        notes::logf("BUFR master table local version numbers differ (first is %hhu, second is %hhu)\n",
                master_table_version_number_local, msg.master_table_version_number_local);
        ++diffs;
    }
    /*
    // TODO: uncomment when we implement encoding BUFR with compression
    if (compression != m->compression)
    {
    notes::logf("BUFR compression differs (first is %d, second is %d)\n",
    compression, m->compression);
    ++diffs;
    }
    */
    if (optional_section.size() != msg.optional_section.size())
    {
        notes::logf("BUFR optional section lenght (first is %zd, second is %zd)\n",
                optional_section.size(), msg.optional_section.size());
        ++diffs;
    }
    if (optional_section != msg.optional_section)
    {
        notes::logf("BUFR optional section contents differ\n");
        ++diffs;
    }
    return diffs;
}

bool BufrBulletin::read(FILE* fd, std::string& buf, const char* fname, long* offset)
{
    /// A BUFR message starts with "BUFR", then the message length encoded in 3 bytes

    // Reset bufr_message data in case this message has been used before
    buf.clear();

    // Seek to start of BUFR data
    if (!seek_past_signature(fd, "BUFR", 4, fname))
        return false;
    buf += "BUFR";
    if (offset) *offset = ftell(fd) - 4;

    // Read the remaining 4 bytes of section 0
    buf.resize(8);
    if (fread((char*)buf.data() + 4, 4, 1, fd) != 1)
    {
        if (fname)
            error_system::throwf("reading BUFR section 0 from %s", fname);
        else
            throw error_system("reading BUFR section 0 from");
    }

    // Read the message length
    int bufrlen = ntohl(*(uint32_t*)(buf.data()+4)) >> 8;
    if (bufrlen < 12)
    {
        if (fname)
            error_consistency::throwf("%s: the size declared by the BUFR message (%d) is less than the minimum of 12", fname, bufrlen);
        else
            error_consistency::throwf("the size declared by the BUFR message (%d) is less than the minimum of 12", bufrlen);
    }

    // Allocate enough space to fit the message
    buf.resize(bufrlen);

    // Read the rest of the BUFR message
    if (fread((char*)buf.data() + 8, bufrlen - 8, 1, fd) != 1)
    {
        if (fname)
            error_system::throwf("reading BUFR message from %s", fname);
        else
            throw error_system("reading BUFR message");
    }

    return true;
}

void BufrBulletin::write(const std::string& buf, FILE* out, const char* fname)
{
    if (fwrite(buf.data(), buf.size(), 1, out) != 1)
    {
        if (fname)
            error_system::throwf("%s: writing %zd bytes", fname, buf.size());
        else
            error_system::throwf("writing %zd bytes", buf.size());
    }
}



/*
 * CrexBulletin
 */

CrexBulletin::CrexBulletin()
{
}

std::unique_ptr<CrexBulletin> CrexBulletin::create()
{
    return unique_ptr<CrexBulletin>(new CrexBulletin);
}

void CrexBulletin::clear()
{
    Bulletin::clear();
    edition_number = 2;
    master_table_number = 0;
    master_table_version_number = 19;
    master_table_version_number_bufr = 19;
    master_table_version_number_local = 0;
    has_check_digit = false;
}

void CrexBulletin::load_tables()
{
    tables.load_crex(CrexTableID(
                edition_number,
                originating_centre, originating_subcentre,
                master_table_number, edition_number,
                master_table_version_number_local, master_table_version_number_bufr));
}

void CrexBulletin::print_details(FILE* out) const
{
    fprintf(out, " CREX details: ed%hhu t%hhu:%hhu:%hhu:%hhu %c\n",
            edition_number,
            master_table_number, master_table_version_number, master_table_version_number_local, master_table_version_number_bufr,
            has_check_digit ? 'C' : '-');
}

unsigned CrexBulletin::diff_details(const Bulletin& bulletin) const
{
    unsigned diffs = Bulletin::diff_details(bulletin);
    const CrexBulletin* cb = dynamic_cast<const CrexBulletin*>(&bulletin);
    if (!cb) throw error_consistency("CrexBulletin::diff_details called with a non-CrexBulletin argument");
    const CrexBulletin& msg = *cb;

    if (edition_number != msg.edition_number)
    {
        notes::logf("CREX edition numbers differ (first is %hhu, second is %hhu)\n",
                edition_number, msg.edition_number);
        ++diffs;
    }
    if (master_table_number != msg.master_table_number)
    {
        notes::logf("CREX master table numbers differ (first is %hhu, second is %hhu)\n",
                master_table_number, msg.master_table_number);
        ++diffs;
    }
    if (master_table_version_number != msg.master_table_version_number)
    {
        notes::logf("CREX master table version numbers differ (first is %hhu, second is %hhu)\n",
                master_table_version_number, msg.master_table_version_number);
        ++diffs;
    }
    if (master_table_version_number_local != msg.master_table_version_number_local)
    {
        notes::logf("CREX master table local version numbers differ (first is %hhu, second is %hhu)\n",
                master_table_version_number_local, msg.master_table_version_number_local);
        ++diffs;
    }
    if (master_table_version_number != msg.master_table_version_number)
    {
        notes::logf("BUFR master table version numbers differ (first is %hhu, second is %hhu)\n",
                master_table_version_number_bufr, msg.master_table_version_number_bufr);
        ++diffs;
    }
    if (has_check_digit != msg.has_check_digit)
    {
        notes::logf("CREX has_check_digit differ (first is %d, second is %d)\n",
                (int)has_check_digit, (int)msg.has_check_digit);
        ++diffs;
    }
    return diffs;
}

bool CrexBulletin::read(FILE* fd, std::string& buf, const char* fname, long* offset)
{
    /*
     * A CREX message starts with "CREX" and ends with "++\r\r\n7777".  Ideally
     * any combination of \r and \n should be supported.
     */

    // Reset crex_message data in case this message has been used before
    buf.clear();

    // Seek to start of CREX data
	if (!seek_past_signature(fd, "CREX++", 6, fname))
		return false;
	buf += "CREX++";
	if (offset) *offset = ftell(fd) - 6;

    // Read until "\+\+(\r|\n)+7777"
	{
		const char* target = "++\r\n7777";
		static const int target_size = 8;
		int got = 0;
		int c;

		errno = 0;
		while (got < 8 && (c = getc(fd)) != EOF)
		{
			if (target[got] == '\r' && (c == '\n' || c == '\r'))
				got++;
			else if (target[got] == '\n' && (c == '\n' || c == '\r'))
				;
			else if (target[got] == '\n' && c == '7')
				got += 2;
			else if (c == target[got])
				got++;
			else
				got = 0;

			buf += (char)c;
		}
		if (errno != 0)
		{
			if (fname)
				error_system::throwf("looking for end of CREX data in %s", fname);
			else
				throw error_system("looking for end of CREX data");
		}

		if (got != target_size)
		{
			if (fname)
				throw error_parse(fname, ftell(fd), "CREX message is incomplete");
			else
				throw error_parse("(unknown)", ftell(fd), "CREX message is incomplete");
		}
	}

	return true;
}

void CrexBulletin::write(const std::string& buf, FILE* out, const char* fname)
{
	if (fwrite(buf.data(), buf.size(), 1, out) != 1)
	{
		if (fname)
			error_system::throwf("%s: writing %zd bytes", fname, buf.size());
		else
			error_system::throwf("writing %zd bytes", buf.size());
	}
	if (fputs("\r\r\n", out) == EOF)
	{
		if (fname)
			error_system::throwf("writing CREX data on %s", fname);
		else
			throw error_system("writing CREX data");
	}
}

}
