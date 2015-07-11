#include "subset.h"
#include "tables.h"
#include "vartable.h"
#include "notes.h"
#include <cstring>
#include "config.h"

using namespace std;

namespace wreport {

Subset::Subset(const Tables& tables) : tables(&tables)
{
    if (!tables.loaded()) throw error_consistency("BUFR/CREX tables not loaded");
    reserve(128);
}

Subset::~Subset() {}

Subset& Subset::operator=(Subset&& s)
{
    if (this == &s) return *this;
    std::vector<Var>::operator=(s);
    tables = s.tables;
    return *this;
}

void Subset::store_variable(const Var& var)
{
	push_back(var);
}

void Subset::store_variable(Var&& var)
{
    emplace_back(move(var));
}

void Subset::store_variable(Varcode code, const Var& var)
{
    Varinfo info = tables->btable->query(code);
    push_back(Var(info, var));
}

void Subset::store_variable_i(Varcode code, int val)
{
    Varinfo info = tables->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_d(Varcode code, double val)
{
    Varinfo info = tables->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_c(Varcode code, const char* val)
{
    Varinfo info = tables->btable->query(code);
    push_back(Var(info, val));
}

void Subset::store_variable_undef(Varcode code)
{
    Varinfo info = tables->btable->query(code);
    push_back(Var(info));
}

void Subset::store_variable_undef(Varinfo info)
{
    push_back(Var(info));
}

void Subset::append_c_with_dpb(Varcode ccode, int count, const char* bitmap)
{
    Varinfo info = tables->get_bitmap(ccode, bitmap);

    // Create the Var with the bitmap
    Var var(info, bitmap);

    // Store the variable in the subset
    store_variable(var);
}

int Subset::append_dpb(Varcode ccode, unsigned size, Varcode attr)
{
	char bitmap[size + 1];
	size_t src, dst;
	size_t count = 0;

    // Scan first 'size' variables checking for the presence of 'attr'
    for (src = 0, dst = 0; src < this->size() && dst < size; ++dst, ++src)
    {
        // Skip extra, special vars
        while (src < this->size() && WR_VAR_F((*this)[src].code()) != 0)
            ++src;

        // Check if the variable has the attribute we want
        if ((*this)[src].enqa(attr) == NULL)
            bitmap[dst] = '-';
		else
		{
			bitmap[dst] = '+';
			++count;
		}
	}
	bitmap[size] = 0;

	// Append the bitmap to the message
	append_c_with_dpb(ccode, size, bitmap);

	return count;
}

void Subset::append_fixed_dpb(Varcode ccode, int size)
{
	char bitmap[size + 1];

	memset(bitmap, '+', size);
	bitmap[size] = 0;

	append_c_with_dpb(ccode, size, bitmap);
}

void Subset::print(FILE* out) const
{
	for (unsigned i = 0; i < size(); ++i)
	{
		fprintf(out, "%d ", i);
		(*this)[i].print(out);
	}
}

unsigned Subset::diff(const Subset& s2) const
{
    // Compare btables
    if (tables->btable->pathname() != s2.tables->btable->pathname())
    {
        notes::logf("B tables differ (first is %s, second is %s)\n",
                tables->btable->pathname().c_str(), s2.tables->btable->pathname().c_str());
        return 1;
    }

    // Compare vars
    if (size() != s2.size())
    {
        notes::logf("Number of variables differ (first is %zd, second is %zd)\n",
                size(), s2.size());
        return 1;
    }
    for (size_t i = 0; i < size(); ++i)
    {
        unsigned diff = (*this)[i].diff(s2[i]);
        if (diff > 0) return diff;
    }
    return 0;
}

}
