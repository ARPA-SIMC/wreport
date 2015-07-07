#ifndef WREPORT_BULLETIN_INTERPETER_H
#define WREPORT_BULLETIN_INTERPETER_H

#include <wreport/bulletin/bitmaps.h>
#include <wreport/bulletin/associated_fields.h>
#include <wreport/opcode.h>
#include <wreport/tables.h>
#include <memory>
#include <stack>

namespace wreport {
struct Vartable;
struct DTable;
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

    /// Current associated field state
    AssociatedField associated_field;

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

protected:
    /**
     * Return a Varinfo for the given Varcode, applying all relevant C
     * modifications that are currently active.
     */
    Varinfo get_varinfo(Varcode code);

public:
    DDSInterpreter(const Tables& tables, const Opcodes& opcodes)
        : tables(tables), associated_field(*tables.btable)
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
     * Request processing, according to \a info, of a data variable.
     *
     * associated_field should be consulted to see if there are also associated
     * fields that need processing.
     */
    virtual void define_variable(Varinfo info);

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
    virtual const Var& define_semantic_variable(Varinfo info);

    /**
     * Request processing of a substituted value corresponding to position \a
     * pos in the list or previous variables
     */
    virtual void define_substituted_value(unsigned pos);

    /**
     * Request processing of an attribute encoded with \a info, related to the
     * variable as position \a pos in the list of previous variables.
     */
    virtual void define_attribute(Varinfo info, unsigned pos);

    /// Request processing of C05yyy raw character data
    virtual void define_raw_character_data(Varcode code);
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
    void c_reuse_last_bitmap(Varcode code) override;
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) override;
    void d_group_begin(Varcode code) override;
    void d_group_end(Varcode code) override;
};


}
}
#endif
