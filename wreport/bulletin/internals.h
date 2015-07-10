#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <wreport/varinfo.h>
#include <wreport/opcodes.h>
#include <wreport/bulletin/interpreter.h>
#include <vector>
#include <memory>
#include <cmath>

namespace wreport {
struct Var;
struct Subset;
struct Bulletin;

namespace bulletin {

/**
 * Base DDSInterpreter specialisation for message encoders that works on a
 * subset at a time
 */
struct UncompressedEncoder : public bulletin::DDSInterpreter
{
    /// Current subset (used to refer to past variables)
    const Subset& current_subset;
    /// Index of the next variable to be visited
    unsigned current_var = 0;

    UncompressedEncoder(const Bulletin& bulletin, unsigned subset_no);
    virtual ~UncompressedEncoder();

    /// Get the next variable, without incrementing current_var
    const Var& peek_var();

    /// Get the next variable, incrementing current_var by 1
    const Var& get_var();

    /// Get the variable at the given position
    const Var& get_var(unsigned pos) const;

    void define_bitmap(unsigned bitmap_size) override;
};

struct UncompressedDecoder : public bulletin::DDSInterpreter
{
    /// Subset where decoded variables go
    Subset& output_subset;

    UncompressedDecoder(Bulletin& bulletin, unsigned subset_no);
    ~UncompressedDecoder();
};

struct CompressedDecoder : public bulletin::DDSInterpreter
{
    Bulletin& output_bulletin;

    CompressedDecoder(Bulletin& bulletin);
    virtual ~CompressedDecoder();
};

}
}

#endif
