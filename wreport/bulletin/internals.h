#ifndef WREPORT_BULLETIN_INTERNALS_H
#define WREPORT_BULLETIN_INTERNALS_H

#include <wreport/varinfo.h>
#include <wreport/opcodes.h>
#include <wreport/bulletin/interpreter.h>
#include <vector>
#include <memory>
#include <cmath>

namespace wreport {
class Var;
class Subset;
class Bulletin;

namespace bulletin {

/**
 * Base Interpreter specialisation for message encoders that works on a
 * subset at a time
 */
struct UncompressedEncoder : public bulletin::Interpreter
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

    void define_variable(Varinfo info) override;
    void define_variable_with_associated_field(Varinfo info) override;
    unsigned define_delayed_replication_factor(Varinfo info) override;
    unsigned define_associated_field_significance(Varinfo info) override;
    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;

    /**
     * Encode a variable.
     *
     * By default, this raises error_unimplemented. For decoders that encode
     * normal variables, delayed replication factors, bitmap delayed
     * replication factors, and associated field significances in the same way,
     * can just override this method and use the default define_*
     * implementations.
     */
    virtual void encode_var(Varinfo info, const Var& var);

    /**
     * Encode an attribute for an associated field.
     *
     * Var is the variable that the associated field refers to. The actual
     * value of the associated field can be looked up using the functions in
     * this->associated_field.
     */
    virtual void encode_associated_field(const Var& var);
};

}
}
#endif
