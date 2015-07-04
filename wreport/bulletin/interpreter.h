#include <wreport/opcode.h>

namespace wreport {
struct Vartable;
struct DTable;

namespace bulletin {
struct Visitor;

struct Interpreter
{
    const DTable& dtable;
    Opcodes opcodes;
    Visitor& visitor;

    Interpreter(const DTable& dtable, Opcodes opcodes, Visitor& visitor)
        : dtable(dtable), opcodes(opcodes), visitor(visitor)
    {
    }

    void run();
};

/**
 * Visitor-style interface for scanning the contents of a data descriptor
 * section.
 *
 * This supports scanning the DDS without looking at the data, so it cannot be
 * used for encoding/decoding, as it cannot access the data that controls
 * decoding such as delayed replicator factors or data descriptor bitmaps.
 *
 * All interface methods have a default implementations that do nothing, so you
 * can override only what you need.
 */
struct Visitor
{
    /**
     * D table to use to expand D groups.
     *
     * This must be provided by the caller
     */
    const DTable* dtable;

    Visitor();
    Visitor(const DTable& dtable);
    virtual ~Visitor();

    /**
     * Notify of a B variable entry
     *
     * @param code
     *   The B variable code
     */
    virtual void b_variable(Varcode code);

    /**
     * Notify of a C modifier
     *
     * Whenever the modifier is a supported one, this is followed by an
     * invocation of one of the specific c_* methods.
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_modifier(Varcode code);

    /**
     * Notify a change of data width
     *
     * @param code
     *   The C modifier code
     * @param change
     *   The width change (positive or negative)
     */
    virtual void c_change_data_width(Varcode code, int change);

    /**
     * Notify a change of data scale
     *
     * @param code
     *   The C modifier code
     * @param change
     *   The scale change (positive or negative)
     */
    virtual void c_change_data_scale(Varcode code, int change);

    /**
     * Notify the declaration of an associated field for the next values.
     *
     * @param code
     *   The C modifier code
     * @param sig_code
     *   The B code of the associated field significance opcode (or 0 to mark
     *   the end of the associated field encoding)
     * @param nbits
     *   The number of bits used for the associated field.
     */
    virtual void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits);

    /**
     * Notify raw character data encoded via a C modifier
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_char_data(Varcode code);

    /**
     * Notify an override of character data length
     *
     * @param code
     *   The C modifier code
     * @param new_length
     *   New length of all following character data (or 0 to reset to default)
     */
    virtual void c_char_data_override(Varcode code, unsigned new_length);

    /**
     * Notify a bitmap for quality information data
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_quality_information_bitmap(Varcode code);

    /**
     * Notify a bitmap for substituted values
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_substituted_value_bitmap(Varcode code);

    /**
     * Notify a substituted value
     *
     * @param code
     *   The C modifier code
     */
    virtual void c_substituted_value(Varcode code);

    /**
     * Notify the length of the following local descriptor
     *
     * @param code
     *   The C modifier code
     * @param desc_code
     *   Local descriptor for which the length is provided
     * @param nbits
     *   Bit size of the data described by \a desc_code
     */
    virtual void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits);

    /**
     * Reuse last defined data present bitmap
     *
     * y is the Y value in the C modifier, currently defined as 0 for reusing
     * the last defined bitmap, and 255 to cancel reuse of the last bitmap.
     */
    virtual void c_reuse_last_bitmap(Varcode code);

    /**
     * Notify a replicated section
     * 
     * @param code
     *   The R replication code
     * @param delayed_code
     *   The delayed replication B code, or 0 if delayed replication is not
     *   used
     * @param ops
     *   The replicated operators
     */
    virtual void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops);

    /**
     * Notify the start of a D group
     *
     * @param code
     *   The D code that is being expanded
     */
    virtual void d_group_begin(Varcode code);

    /**
     * Notify the end of a D group
     *
     * @param code
     *   The D code that has just been expanded
     */
    virtual void d_group_end(Varcode code);

    /**
     * Notify an increase of scale, reference value and data width
     *
     * @param code
     *   The C modifier code
     * @param change
     *   The increase, to be handled according to table C, X=7
     */
    virtual void c_increase_scale_ref_width(Varcode code, int change);
};

/**
 * opcode::Visitor that pretty-prints the opcodes using indentation to show
 * structure
 */
class Printer : public Visitor
{
protected:
    /**
     * Print line lead (indentation and formatted code)
     *
     * @param code
     *   Code to format in the line lead
     */
    void print_lead(Varcode code);

public:
    /**
     * Output stream.
     *
     * It defaults to stdout, but it can be set to any FILE* stream
     */
    FILE* out;

    /**
     * Table used to get variable descriptions (optional).
     *
     * It defaults to NULL, but if it is set, the output will contain
     * descriptions of B variable entries
     */
    const Vartable* btable;

    /**
     * Current indent level
     *
     * It defaults to 0 in a newly created Printer. You can set it to some
     * other value to indent all the output by the given amount of spaces
     */
    unsigned indent;

    /// How many spaces in an indentation level
    unsigned indent_step;

    Printer();
    virtual void b_variable(Varcode code);
    virtual void c_modifier(Varcode code);
    virtual void c_change_data_width(Varcode code, int change);
    virtual void c_change_data_scale(Varcode code, int change);
    virtual void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits);
    virtual void c_char_data(Varcode code);
    virtual void c_char_data_override(Varcode code, unsigned new_length);
    virtual void c_quality_information_bitmap(Varcode code);
    virtual void c_substituted_value_bitmap(Varcode code);
    virtual void c_substituted_value(Varcode code);
    virtual void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits);
    virtual void c_reuse_last_bitmap(Varcode code);
    virtual void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops);
    virtual void d_group_begin(Varcode code);
    virtual void d_group_end(Varcode code);
};


}
}
