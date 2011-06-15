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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

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
    run_dds(printer);
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

DDSExecutor::DDSExecutor() : btable(0), current_subset(0) {}
DDSExecutor::~DDSExecutor() {}
void DDSExecutor::start_subset(unsigned subset_no, const Subset& current_subset)
{
    this->current_subset = &current_subset;
}
void DDSExecutor::push_repetition(unsigned length, unsigned count) {}
void DDSExecutor::start_repetition() {}
void DDSExecutor::pop_repetition() {}
void DDSExecutor::push_dcode(Varcode code) {}
void DDSExecutor::pop_dcode() {}

BaseDDSExecutor::BaseDDSExecutor(Bulletin& bulletin)
    : bulletin(bulletin), current_subset_no(0)
{
}

const Var& BaseDDSExecutor::get_var()
{
    const Var& res = get_var(current_var);
    ++current_var;
    return res;
}

const Var& BaseDDSExecutor::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset->size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return (*current_subset)[var_pos];
}

void BaseDDSExecutor::start_subset(unsigned subset_no, const Subset& current_subset)
{
    if (subset_no >= bulletin.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, bulletin.subsets.size());
    this->current_subset = &(bulletin.subsets[subset_no]);
    current_subset_no = subset_no;
    current_var = 0;
}

unsigned BaseDDSExecutor::subset_size()
{
    if (!current_subset)
        return 0;
    return current_subset->size();
}

bool BaseDDSExecutor::is_special_var(unsigned var_pos)
{
    if (!current_subset) return true;
    if (var_pos >= current_subset->size()) return true;
    return WR_VAR_F((*current_subset)[var_pos].code()) != 0;
}

const Var* BaseDDSExecutor::get_bitmap()
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return &var;
}

void BaseDDSExecutor::encode_associated_field(unsigned bit_count, unsigned significance)
{
}


ConstBaseDDSExecutor::ConstBaseDDSExecutor(const Bulletin& bulletin)
    : bulletin(bulletin), current_subset_no(0)
{
}

const Var& ConstBaseDDSExecutor::get_var()
{
    const Var& res = get_var(current_var);
    ++current_var;
    return res;
}

const Var& ConstBaseDDSExecutor::get_var(unsigned var_pos) const
{
    unsigned max_var = current_subset->size();
    if (var_pos >= max_var)
        error_consistency::throwf("requested variable #%u out of a maximum of %u in subset %u",
                var_pos, max_var, current_subset_no);
    return (*current_subset)[var_pos];
}

void ConstBaseDDSExecutor::start_subset(unsigned subset_no, const Subset& current_subset)
{
    DDSExecutor::start_subset(subset_no, current_subset);
    if (subset_no >= bulletin.subsets.size())
        error_consistency::throwf("requested subset #%u out of a maximum of %zd", subset_no, bulletin.subsets.size());
    current_subset_no = subset_no;
    current_var = 0;
}

unsigned ConstBaseDDSExecutor::subset_size()
{
    if (!current_subset)
        return 0;
    return current_subset->size();
}

bool ConstBaseDDSExecutor::is_special_var(unsigned var_pos)
{
    if (!current_subset) return true;
    if (var_pos >= current_subset->size()) return true;
    return WR_VAR_F((*current_subset)[var_pos].code()) != 0;
}

const Var* ConstBaseDDSExecutor::get_bitmap()
{
    const Var& var = get_var();
    if (WR_VAR_F(var.code()) != 2)
        error_consistency::throwf("variable at %u is %01d%02d%03d and not a data present bitmap",
                current_var-1, WR_VAR_F(var.code()), WR_VAR_X(var.code()), WR_VAR_Y(var.code()));
    return &var;
}

void ConstBaseDDSExecutor::encode_associated_field(unsigned bit_count, unsigned significance)
{
}

}

}

/* vim:set ts=4 sw=4: */
