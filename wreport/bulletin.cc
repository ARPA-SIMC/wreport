/*
 * wreport/bulletin - Decoded weather bulletin
 *
 * Copyright (C) 2005--2011  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include <config.h>

#include "error.h"
#include "opcode.h"
#include "bulletin.h"
#include "bulletin/dds-printer.h"
#include "bulletin/buffers.h"
#include "notes.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <netinet/in.h>

// #define TRACE_INTERPRETER

#ifdef TRACE_INTERPRETER
#define TRACE(...) fprintf(stderr, __VA_ARGS__)
#define IFTRACE if (1)
#else
#define TRACE(...) do { } while (0)
#define IFTRACE if (0)
#endif

using namespace std;

namespace wreport {

Bulletin::Bulletin() : btable(0), dtable(0) {}
Bulletin::~Bulletin() {}

void Bulletin::clear()
{
	datadesc.clear();
	subsets.clear();
	type = subtype = localsubtype = edition = master_table_number = 0;
	rep_year = rep_month = rep_day = rep_hour = rep_minute = rep_second = 0;
	btable = 0;
	dtable = 0;
}

Subset& Bulletin::obtain_subset(unsigned subsection)
{
	while (subsection >= subsets.size())
		subsets.push_back(Subset(btable));
	return subsets[subsection];
}

const Subset& Bulletin::subset(unsigned subsection) const
{
	if (subsection >= subsets.size())
		error_notfound::throwf("Requested subset %u but there are only %zd available",
				subsection, subsets.size());
	return subsets[subsection];
}


BufrBulletin::BufrBulletin()
	: optional_section_length(0), optional_section(0), raw_details(0), codec_options(0)
{
}

BufrBulletin::~BufrBulletin()
{
	if (optional_section) delete[] optional_section;
    if (raw_details) delete raw_details;
}

void BufrBulletin::clear()
{
	Bulletin::clear();
	centre = subcentre = master_table = local_table = 0;
	compression = update_sequence_number = optional_section_length = 0;
	if (optional_section) delete[] optional_section;
	optional_section = 0;
}

void BufrBulletin::load_tables()
{
	char id[30];
	int ce = centre;
	int sc = subcentre;
	int mt = master_table;
	int lt = local_table;

/* fprintf(stderr, "ce %d sc %d mt %d lt %d\n", ce, sc, mt, lt); */

	vector<string> ids;
	for (int i = 0; i < 3; ++i)
	{
		if (i == 1)
		{
			// Default to WMO tables if the first
			// attempt with local tables failed
			/* fprintf(stderr, "FALLBACK from %d %d %d %d to 0 %d 0 0\n", sc, ce, mt, lt, mt); */
			ce = sc = lt = 0;
		} else if (i == 2 && mt < 14) {
			// Default to the latest WMO table that
			// we have if the previous attempt has
			// failed
			/* fprintf(stderr, "FALLBACK from %d %d %d %d to 0 14 0 0\n", sc, ce, mt, lt); */
			mt = 14;
		}
		switch (edition)
		{
			case 2:
				sprintf(id, "B%05d%02d%02d", ce, mt, lt);
				ids.push_back(id);
			case 3:
				sprintf(id, "B00000%03d%03d%02d%02d",
						0, ce, mt, lt);
				/* Some tables used by BUFR3 are
				 * distributed using BUFR4 names
				 */
				ids.push_back(id);
				sc = 0;
			case 4:
				sprintf(id, "B00%03d%04d%04d%03d%03d",
						0, sc, ce, mt, lt);
				ids.push_back(id);
				break;
			default:
				error_consistency::throwf("BUFR edition number is %d but I can only load tables for 2, 3 or 4", edition);
		}
	}

	btable = Vartable::get(Vartable::find_table(ids));
	/* TRACE(" -> loaded B table %s\n", id); */

	for (vector<string>::iterator i = ids.begin(); i != ids.end(); ++i)
		(*i)[0] = 'D';
	dtable = DTable::get(Vartable::find_table(ids));
	/* TRACE(" -> loaded D table %s\n", id); */
}

bulletin::BufrInput& BufrBulletin::reset_raw_details(const std::string& buf)
{
    if (raw_details == 0)
        raw_details = new bulletin::BufrInput(buf);
    else
        raw_details->reset(buf);
    return *raw_details;
}

/*
implemented in bufr_decoder.cc
void BufrBulletin::decode_header(const Rawmsg& raw)
{
}

void BufrBulletin::decode(const Rawmsg& raw)
{
}

implemented in bufr_encoder.cc
void BufrBulletin::encode(std::string& buf) const
{
}
*/

void CrexBulletin::clear()
{
	Bulletin::clear();
	table = has_check_digit = 0;
}

void CrexBulletin::load_tables()
{
	char id[30];
/* fprintf(stderr, "ce %d sc %d mt %d lt %d\n", ce, sc, mt, lt); */

	sprintf(id, "B%02d%02d%02d", master_table_number, edition, table);

	btable = Vartable::get(id);
	/* TRACE(" -> loaded B table %s\n", id); */

	id[0] = 'D';
	dtable = DTable::get(id);
	/* TRACE(" -> loaded D table %s\n", id); */
}

/*
implemented in crex_decoder.cc
void CrexBulletin::decode_header(const Rawmsg& raw)
{
}

void CrexBulletin::decode(const Rawmsg& raw)
{
}

implemented in crex_encoder.cc
void CrexBulletin::encode(std::string& buf) const
{
}
*/

void Bulletin::visit_datadesc(opcode::Visitor& e) const
{
    Opcodes(datadesc).visit(e, *dtable);
}

void Bulletin::visit(bulletin::Visitor& out) const
{
    out.btable = btable;
    out.dtable = dtable;

    /* Run all the subsets, uncompressed */
    for (unsigned i = 0; i < subsets.size(); ++i)
    {
        TRACE("visit: start encoding subset %u\n", i);
        /* Encode the data of this subset */
        out.do_start_subset(i, subsets[i]);
        Opcodes(datadesc).visit(out);
        TRACE("visit: done encoding subset %u\n", i);
    }
}

void Bulletin::print(FILE* out) const
{
	fprintf(out, "%s ed%d %d:%d:%d %04d-%02d-%02d %02d:%02d:%02d %zd subsets\n",
		encoding_name(), edition,
		type, subtype, localsubtype,
		rep_year, rep_month, rep_day, rep_hour, rep_minute, rep_second,
		subsets.size());
	fprintf(out, " Tables: %s %s\n",
		btable ? btable->id().c_str() : "(not loaded)",
		dtable ? dtable->id().c_str() : "(not loaded)");
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
    fprintf(out, "%s ed%d %d:%d:%d %04d-%02d-%02d %02d:%02d:%02d %zd subsets\n",
            encoding_name(), edition,
            type, subtype, localsubtype,
            rep_year, rep_month, rep_day, rep_hour, rep_minute, rep_second,
            subsets.size());
    fprintf(out, " Tables: %s %s\n",
            btable ? btable->id().c_str() : "(not loaded)",
            dtable ? dtable->id().c_str() : "(not loaded)");
    fprintf(out, " Data descriptors:\n");
    for (vector<Varcode>::const_iterator i = datadesc.begin(); i != datadesc.end(); ++i)
        fprintf(out, "  %d%02d%03d\n", WR_VAR_F(*i), WR_VAR_X(*i), WR_VAR_Y(*i));
    print_details(out);
    fprintf(out, " Variables:\n");
    bulletin::DDSPrinter printer(*this, out);
    visit(printer);
}

void Bulletin::print_details(FILE* out) const {}

void BufrBulletin::print_details(FILE* out) const
{
	fprintf(out, " BUFR details: c%d/%d mt%d lt%d co%d usn%d osl%d\n",
			centre, subcentre, master_table, local_table,
			compression, update_sequence_number, optional_section_length);
}

void CrexBulletin::print_details(FILE* out) const
{
	fprintf(out, " CREX details: T%02d%02d%02d cd%d\n", master_table_number, edition, table, has_check_digit);
}

void Bulletin::print_datadesc(FILE* out, unsigned indent) const
{
    opcode::Printer printer;
    printer.out = out;
    printer.btable = btable;
    printer.dtable = dtable;
    printer.indent = indent;

    Opcodes(datadesc).visit(printer);
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
    if (master_table_number != msg.master_table_number)
    {
        notes::logf("Master tables numbers differ (first is %d, second is %d)\n",
                master_table_number, msg.master_table_number);
        ++diffs;
    }
    if (type != msg.type)
    {
        notes::logf("Template types differ (first is %d, second is %d)\n",
                type, msg.type);
        ++diffs;
    }
    if (subtype != msg.subtype)
    {
        notes::logf("Template subtypes differ (first is %d, second is %d)\n",
                subtype, msg.subtype);
        ++diffs;
    }
    if (localsubtype != msg.localsubtype)
    {
        notes::logf("Template local subtypes differ (first is %d, second is %d)\n",
                localsubtype, msg.localsubtype);
        ++diffs;
    }
    if (edition != msg.edition)
    {
        notes::logf("Edition numbers differ (first is %d, second is %d)\n",
                edition, msg.edition);
        ++diffs;
    }
    if (rep_year != msg.rep_year)
    {
        notes::logf("Reference years differ (first is %d, second is %d)\n",
                rep_year, msg.rep_year);
        ++diffs;
    }
    if (rep_month != msg.rep_month)
    {
        notes::logf("Reference months differ (first is %d, second is %d)\n",
                rep_month, msg.rep_month);
        ++diffs;
    }
    if (rep_day != msg.rep_day)
    {
        notes::logf("Reference days differ (first is %d, second is %d)\n",
                rep_day, msg.rep_day);
        ++diffs;
    }
    if (rep_hour != msg.rep_hour)
    {
        notes::logf("Reference hours differ (first is %d, second is %d)\n",
                rep_hour, msg.rep_hour);
        ++diffs;
    }
    if (rep_minute != msg.rep_minute)
    {
        notes::logf("Reference minutes differ (first is %d, second is %d)\n",
                rep_minute, msg.rep_minute);
        ++diffs;
    }
    if (rep_second != msg.rep_second)
    {
        notes::logf("Reference seconds differ (first is %d, second is %d)\n",
                rep_second, msg.rep_second);
        ++diffs;
    }
    if (btable == NULL && msg.btable != NULL)
    {
        notes::logf("First message did not load B btables, second message has %s\n",
                msg.btable->id().c_str());
        ++diffs;
    } else if (btable != NULL && msg.btable == NULL) {
        notes::logf("Second message did not load B btables, first message has %s\n",
                btable->id().c_str());
        ++diffs;
    } else if (btable != NULL && msg.btable != NULL && btable->id() != msg.btable->id()) {
        notes::logf("B tables differ (first has %s, second has %s)\n",
                btable->id().c_str(), msg.btable->id().c_str());
        ++diffs;
    }
    if (dtable == NULL && msg.dtable != NULL)
    {
        notes::logf("First message did not load B dtable, second message has %s\n",
                msg.dtable->id().c_str());
        ++diffs;
    } else if (dtable != NULL && msg.dtable == NULL) {
        notes::logf("Second message did not load B dtable, first message has %s\n",
                dtable->id().c_str());
        ++diffs;
    } else if (dtable != NULL && msg.dtable != NULL && dtable->id() != msg.dtable->id()) {
        notes::logf("D tables differ (first has %s, second has %s)\n",
                dtable->id().c_str(), msg.dtable->id().c_str());
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

unsigned BufrBulletin::diff_details(const Bulletin& msg) const
{
    unsigned diffs = Bulletin::diff_details(msg);
    const BufrBulletin* m = dynamic_cast<const BufrBulletin*>(&msg);
    if (!m) throw error_consistency("BufrBulletin::diff_details called with a non-BufrBulletin argument");

    if (centre != m->centre)
    {
        notes::logf("BUFR centres differ (first is %d, second is %d)\n",
                centre, m->centre);
        ++diffs;
    }
    if (subcentre != m->subcentre)
    {
        notes::logf("BUFR subcentres differ (first is %d, second is %d)\n",
                subcentre, m->subcentre);
        ++diffs;
    }
    if (master_table != m->master_table)
    {
        notes::logf("BUFR master tables differ (first is %d, second is %d)\n",
                master_table, m->master_table);
        ++diffs;
    }
    if (local_table != m->local_table)
    {
        notes::logf("BUFR local tables differ (first is %d, second is %d)\n",
                local_table, m->local_table);
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
    if (update_sequence_number != m->update_sequence_number)
    {
        notes::logf("BUFR update sequence numbers differ (first is %d, second is %d)\n",
                update_sequence_number, m->update_sequence_number);
        ++diffs;
    }
    if (optional_section_length != m->optional_section_length)
    {
        notes::logf("BUFR optional section lenght (first is %d, second is %d)\n",
                optional_section_length, m->optional_section_length);
        ++diffs;
    }
    if (optional_section_length != 0)
    {
        if (memcmp(optional_section, m->optional_section, optional_section_length) != 0)
        {
            notes::logf("BUFR optional section contents differ\n");
            ++diffs;
        }
    }
    return diffs;
}

unsigned CrexBulletin::diff_details(const Bulletin& msg) const
{
    unsigned diffs = Bulletin::diff_details(msg);
    const CrexBulletin* m = dynamic_cast<const CrexBulletin*>(&msg);
    if (!m) throw error_consistency("CrexBulletin::diff_details called with a non-CrexBulletin argument");

    if (table != m->table)
    {
        notes::logf("CREX local tables differ (first is %d, second is %d)\n",
                table, m->table);
        ++diffs;
    }
    if (has_check_digit != m->has_check_digit)
    {
        notes::logf("CREX has_check_digit differ (first is %d, second is %d)\n",
                has_check_digit, m->has_check_digit);
        ++diffs;
    }
    return diffs;
}

static bool seek_past_signature(FILE* fd, const char* sig, unsigned sig_len, const char* fname)
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

bool BufrBulletin::read(FILE* fd, std::string& buf, const char* fname, long* offset)
{
	/* A BUFR message is easy to just read: it starts with "BUFR", then the
	 * message length encoded in 3 bytes */

	// Reset bufr_message data in case this message has been used before
	buf.clear();

	/* Seek to start of BUFR data */
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

	/* Read the message length */
	int bufrlen = ntohl(*(uint32_t*)(buf.data()+4)) >> 8;
	if (bufrlen < 12)
	{
		if (fname)
			error_consistency::throwf("%s: the size declared by the BUFR message (%d) is less than the minimum of 12", fname, bufrlen);
		else
			error_consistency::throwf("the size declared by the BUFR message (%d) is less than the minimum of 12", bufrlen);
	}

	/* Allocate enough space to fit the message */
	buf.resize(bufrlen);

	/* Read the rest of the BUFR message */
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

bool CrexBulletin::read(FILE* fd, std::string& buf, const char* fname, long* offset)
{
/*
 * The CREX message starts with "CREX" and ends with "++\r\r\n7777".  Ideally
 * any combination of \r and \n should be supported.
 */
	/* Reset crex_message data in case this message has been used before */
	buf.clear();

	/* Seek to start of CREX data */
	if (!seek_past_signature(fd, "CREX++", 6, fname))
		return false;
	buf += "CREX++";
	if (offset) *offset = ftell(fd) - 4;

	/* Read until "\+\+(\r|\n)+7777" */
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

#if 0
std::auto_ptr<Bulletin> Bulletin::create(dballe::Encoding encoding)
{
	std::auto_ptr<bufrex::Bulletin> res;
	switch (encoding)
	{
		case BUFR: res.reset(new BufrBulletin); break;
		case CREX: res.reset(new CrexBulletin); break;
		default: error_consistency::throwf("the bufrex library does not support encoding %s (only BUFR and CREX are supported)",
					 encoding_name(encoding));
	}
	return res;
}
#endif

namespace bulletin {

Bitmap::Bitmap() : bitmap(0) {}
Bitmap::~Bitmap() {}

void Bitmap::reset()
{
    bitmap = 0;
    old_anchor = 0;
    refs.clear();
    iter = refs.rend();
}

void Bitmap::init(const Var& bitmap, const Subset& subset, unsigned anchor)
{
    this->bitmap = &bitmap;
    refs.clear();

    // From the specs it looks like bitmaps refer to all data that precedes
    // the C operator that defines or uses the bitmap, but from the data
    // samples that we have it look like when multiple bitmaps are present,
    // they always refer to the same set of variables. For this reason we
    // remember the first anchor point that we see and always refer the
    // other bitmaps that we see to it.
    if (old_anchor)
        anchor = old_anchor;
    else
        old_anchor = anchor;

    unsigned b_cur = bitmap.info()->len;
    unsigned s_cur = anchor;
    if (b_cur == 0) throw error_consistency("data present bitmap has length 0");
    if (s_cur == 0) throw error_consistency("data present bitmap is anchored at start of subset");

    while (true)
    {
        --b_cur;
        --s_cur;
        while (WR_VAR_F(subset[s_cur].code()) != 0)
        {
            if (s_cur == 0) throw error_consistency("bitmap refers to variables before the start of the subset");
            --s_cur;
        }

        if (bitmap.value()[b_cur] == '+')
            refs.push_back(s_cur);

        if (b_cur == 0)
            break;
        if (s_cur == 0)
            throw error_consistency("bitmap refers to variables before the start of the subset");
    }

    iter = refs.rbegin();
}

bool Bitmap::eob() const { return iter == refs.rend(); }
unsigned Bitmap::next() { unsigned res = *iter; ++iter; return res; }



Visitor::Visitor() : btable(0), current_subset(0) {}
Visitor::~Visitor() {}

Varinfo Visitor::get_varinfo(Varcode code)
{
    Varinfo peek = btable->query(code);

    if (!c_scale_change && !c_width_change && !c_string_len_override)
        return peek;

    int scale = peek->scale;
    if (c_scale_change)
    {
        TRACE("get_info:applying %d scale change\n", c_scale_change);
        scale += c_scale_change;
    }

    int bit_len = peek->bit_len;
    if (peek->is_string() && c_string_len_override)
    {
        TRACE("get_info:overriding string to %d bytes\n", c_string_len_override);
        bit_len = c_string_len_override * 8;
    }
    else if (c_width_change)
    {
        TRACE("get_info:applying %d width change\n", c_width_change);
        bit_len += c_width_change;
    }

    TRACE("get_info:requesting alteration scale:%d, bit_len:%d\n", scale, bit_len);
    return btable->query_altered(code, scale, bit_len);
}

void Visitor::b_variable(Varcode code)
{
    Varinfo info = get_varinfo(code);
    // Choose which value we should encode
    if (WR_VAR_F(code) == 0 && WR_VAR_X(code) == 33 && !bitmap.eob())
    {
        // Attribute of the variable pointed by the bitmap
        unsigned target = bitmap.next();
        TRACE("Encode attribute %01d%02d%03d subset pos %u\n",
                WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code), target);
        do_attr(info, target, code);
    } else {
        // Proper variable
        TRACE("Encode variable %01d%02d%03d\n",
                WR_VAR_F(info->var), WR_VAR_X(info->var), WR_VAR_Y(info->var));
        if (c04_bits > 0)
            do_associated_field(c04_bits, c04_meaning);
        do_var(info);
        ++data_pos;
    }
}

void Visitor::c_modifier(Varcode code)
{
    TRACE("C DATA %01d%02d%03d\n", WR_VAR_F(code), WR_VAR_X(code), WR_VAR_Y(code));
}

void Visitor::c_change_data_width(Varcode code, int change)
{
    TRACE("Set width change from %d to %d\n", c_width_change, change);
    c_width_change = change;
}

void Visitor::c_change_data_scale(Varcode code, int change)
{
    TRACE("Set scale change from %d to %d\n", c_scale_change, change);
    c_scale_change = change;
}

void Visitor::c_associated_field(Varcode code, Varcode sig_code, unsigned nbits)
{
    // Add associated field
    TRACE("Set C04 bits to %d\n", WR_VAR_Y(code));
    // FIXME: nested C04 modifiers are not currently implemented
    if (WR_VAR_Y(code) && c04_bits)
        throw error_unimplemented("nested C04 modifiers are not yet implemented");
    if (WR_VAR_Y(code) > 32)
        error_unimplemented::throwf("C04 modifier wants %d bits but only at most 32 are supported", WR_VAR_Y(code));
    if (WR_VAR_Y(code))
    {
        // Get encoding informations for this associated_field_significance
        Varinfo info = btable->query(WR_VAR(0, 31, 21));

        // Encode B31021
        Var var = do_semantic_var(info);
        c04_meaning = var.enqi();
        ++data_pos;
    }
    c04_bits = WR_VAR_Y(code);
}

void Visitor::c_char_data(Varcode code)
{
    do_char_data(code);
}

void Visitor::c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits)
{
    // Length of next local descriptor
    if (WR_VAR_Y(code) > 32)
        error_unimplemented::throwf("C06 modifier found for %d bits but only at most 32 are supported", WR_VAR_Y(code));
    if (WR_VAR_Y(code))
    {
        bool skip = true;
        if (btable->contains(desc_code))
        {
            Varinfo info = get_varinfo(desc_code);
            if (info->bit_len == WR_VAR_Y(code))
            {
                // If we can resolve the descriptor and the size is the
                // same, attempt decoding
                do_var(info);
                skip = false;
            }
        }
        if (skip)
        {
            MutableVarinfo info(MutableVarinfo::create_singleuse());
            info->set(code, "UNKNOWN LOCAL DESCRIPTOR", "UNKNOWN", 0, 0,
                    ceil(log10(exp2(WR_VAR_Y(code)))), 0, WR_VAR_Y(code), VARINFO_FLAG_STRING);
            do_var(info);
        }
        ++data_pos;
    }
}

void Visitor::c_char_data_override(Varcode code, unsigned new_length)
{
    IFTRACE {
        if (new_length)
            TRACE("decode_c_data:character size overridden to %d chars for all fields\n", new_length);
        else
            TRACE("decode_c_data:character size overridde end\n");
    }
    c_string_len_override = new_length;
}

void Visitor::c_quality_information_bitmap(Varcode code)
{
    // Quality information
    if (WR_VAR_Y(code) != 0)
        error_consistency::throwf("C modifier %d%02d%03d not yet supported",
                    WR_VAR_F(code),
                    WR_VAR_X(code),
                    WR_VAR_Y(code));
    want_bitmap = true;
}

void Visitor::c_substituted_value_bitmap(Varcode code)
{
    want_bitmap = true;
}

void Visitor::c_substituted_value(Varcode code)
{
    if (bitmap.bitmap == NULL)
        error_consistency::throwf("found C23255 with no active bitmap");
    if (bitmap.eob())
        error_consistency::throwf("found C23255 while at the end of active bitmap");
    unsigned target = bitmap.next();
    // Use the details of the corrisponding variable for decoding
    Varinfo info = (*current_subset)[target].info();
    // Encode the value
    do_attr(info, target, info->var);
}

/* If using delayed replication and count is not -1, use count for the delayed
 * replication factor; else, look for a delayed replication factor among the
 * input variables */
void Visitor::r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    //int group = WR_VAR_X(code);
    unsigned count = WR_VAR_Y(code);

    IFTRACE{
        TRACE("bufr_message_encode_r_data %01d%02d%03d %d %d: items: ",
                WR_VAR_F(ops.head()), WR_VAR_X(ops.head()), WR_VAR_Y(ops.head()), group, count);
        ops.print(stderr);
        TRACE("\n");
    }

    if (want_bitmap)
    {
        if (count == 0 && delayed_code == 0)
            delayed_code = WR_VAR(0, 31, 12);
        const Var* bitmap_var = do_bitmap(code, delayed_code, ops);
        bitmap.init(*bitmap_var, *current_subset, data_pos);
        if (delayed_code)
            ++data_pos;
        want_bitmap = false;
    } else {
        if (count == 0)
        {
            Varinfo info = btable->query(delayed_code ? delayed_code : WR_VAR(0, 31, 12));
            Var var = do_semantic_var(info);
            count = var.enqi();
            ++data_pos;
        }
        TRACE("encode_r_data %d items %d times%s\n", group, count, delayed_code ? " (delayed)" : "");
        IFTRACE {
            TRACE("Repeat opcodes: ");
            ops.print(stderr);
            TRACE("\n");
        }

        // encode_data_section on it `count' times
        for (unsigned i = 0; i < count; ++i)
        {
            do_start_repetition(i);
            ops.visit(*this);
        }
    }
}

void Visitor::do_start_subset(unsigned subset_no, const Subset& current_subset)
{
    this->current_subset = &current_subset;

    c_scale_change = 0;
    c_width_change = 0;
    c_string_len_override = 0;
    bitmap.reset();
    c04_bits = 0;
    c04_meaning = 63;
    want_bitmap = false;
    data_pos = 0;
}

void Visitor::do_start_repetition(unsigned idx) {}



BaseVisitor::BaseVisitor(Bulletin& bulletin)
    : bulletin(bulletin), current_subset_no(0)
{
}

Var& BaseVisitor::get_var()
{
    Var& res = get_var(current_var);
    ++current_var;
    return res;
}

Var& BaseVisitor::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset->size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return bulletin.subsets[current_subset_no][var_pos];
}

void BaseVisitor::do_start_subset(unsigned subset_no, const Subset& current_subset)
{
    Visitor::do_start_subset(subset_no, current_subset);
    if (subset_no >= bulletin.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, bulletin.subsets.size());
    this->current_subset = &(bulletin.subsets[subset_no]);
    current_subset_no = subset_no;
    current_var = 0;
}

const Var* BaseVisitor::do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return &var;
}


ConstBaseVisitor::ConstBaseVisitor(const Bulletin& bulletin)
    : bulletin(bulletin), current_subset_no(0)
{
}

const Var& ConstBaseVisitor::get_var()
{
    const Var& res = get_var(current_var);
    ++current_var;
    return res;
}

const Var& ConstBaseVisitor::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset->size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return (*current_subset)[var_pos];
}

void ConstBaseVisitor::do_start_subset(unsigned subset_no, const Subset& current_subset)
{
    Visitor::do_start_subset(subset_no, current_subset);
    if (subset_no >= bulletin.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, bulletin.subsets.size());
    current_subset_no = subset_no;
    current_var = 0;
}

const Var* ConstBaseVisitor::do_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops)
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return &var;
}

}

}

/* vim:set ts=4 sw=4: */
