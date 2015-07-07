#ifndef WREPORT_BULLETIN_INTERPETER_H
#define WREPORT_BULLETIN_INTERPETER_H

#include <bulletin/bitmaps.h>
#include <wreport/opcode.h>
#include <memory>
#include <stack>

namespace wreport {
struct Vartable;
struct DTable;
struct Tables;
struct Var;

namespace bulletin {
struct Visitor;

/**
 * Interpreter for data descriptor sections.
 *
 * By default, the interpreter goes through all the motions without doing
 * anything. To provide actual functionality, subclass the interpreter and
 * override the various virtual methods.
 */
struct DDSInterpreter
{
    const Tables& tables;
    std::stack<Opcodes> opcode_stack;

    /// Bitmap iteration
    Bitmaps bitmaps;

    /// Current value of scale change from C modifier
    int c_scale_change = 0;

    /// Current value of width change from C modifier
    int c_width_change = 0;

    /// Increase of scale, reference value and data width
    int c_scale_ref_width_increase = 0;

    /**
     * Current value of string length override from C08 modifiers (0 for no
     * override)
     */
    int c_string_len_override = 0;


    DDSInterpreter(const Tables& tables, const Opcodes& opcodes)
        : tables(tables)
    {
        opcode_stack.push(opcodes);
    }

    virtual ~DDSInterpreter()
    {
    }

    DDSInterpreter(const DDSInterpreter&) = delete;
    DDSInterpreter& operator=(const DDSInterpreter&) = delete;

    /// Run the interpreter
    void run();

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
    virtual void c_modifier(Varcode code, Opcodes& next);

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
     * Request processing of a data present bitmap.
     *
     * @param code
     *   The C modifier code that defines the bitmap
     * @param rep_code
     *   The R replicator that defines the bitmap
     * @param delayed_code
     *   The B delayed replicator that defines the bitmap length (it is 0 if
     *   the length is encoded in the YYY part of rep_code
     * @param ops
     *   The replicated opcodes that define the bitmap
     * @returns
     *   The bitmap that has been processed.
     */
    virtual void define_bitmap(Varcode rep_code, Varcode delayed_code, const Opcodes& ops);

    /**
     * Request processing, according to \a info, of a data variabile that is
     * significant for controlling the encoding process.
     *
     * This means that the variable has always the same value on all datasets
     * (in case of compressed datasets), and that the interpreter needs to know
     * its value.
     *
     * @returns a copy of the variable
     */
    virtual const Var& define_semantic_var(Varinfo info);
};


/**
 * opcode::Visitor that pretty-prints the opcodes using indentation to show
 * structure
 */
class Printer : public DDSInterpreter
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
     * Current indent level
     *
     * It defaults to 0 in a newly created Printer. You can set it to some
     * other value to indent all the output by the given amount of spaces
     */
    unsigned indent;

    /// How many spaces in an indentation level
    unsigned indent_step;

    Printer(const Tables& tables, const Opcodes& opcodes);

    void b_variable(Varcode code) override;
    void c_modifier(Varcode code, Opcodes& next) override;
    void c_associated_field(Varcode code, Varcode sig_code, unsigned nbits) override;
    void c_char_data(Varcode code) override;
    void c_substituted_value_bitmap(Varcode code) override;
    void c_substituted_value(Varcode code) override;
    void c_local_descriptor(Varcode code, Varcode desc_code, unsigned nbits) override;
    void c_reuse_last_bitmap(Varcode code) override;
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) override;
    void d_group_begin(Varcode code) override;
    void d_group_end(Varcode code) override;
};


}
}
#endif
