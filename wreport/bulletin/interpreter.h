#ifndef WREPORT_BULLETIN_INTERPETER_H
#define WREPORT_BULLETIN_INTERPETER_H

#include <wreport/bulletin/bitmaps.h>
#include <wreport/bulletin/associated_fields.h>
#include <wreport/opcodes.h>
#include <wreport/tables.h>
#include <memory>
#include <stack>

namespace wreport {
struct Vartable;
struct DTable;
struct Var;

namespace bulletin {

/**
 * Interpreter for data descriptor sections.
 *
 * By default, the interpreter goes through all the motions without doing
 * anything. To provide actual functionality, subclass the interpreter and
 * override the various virtual methods.
 */
struct Interpreter
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
    Interpreter(const Tables& tables, const Opcodes& opcodes);
    virtual ~Interpreter();

    Interpreter(const Interpreter&) = delete;
    Interpreter& operator=(const Interpreter&) = delete;

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
     * Handle a replicated section
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
     * Handle a replicated section which defines a bitmap
     */
    virtual void r_bitmap(Varcode code, Varcode delayed_code, const Opcodes& ops);

    /**
     * Executes a repetition of the opcodes on top of the stack.
     *
     * By default it just calls run(), but it can be overridden to execute
     * operations before and after.
     *
     * @param cur
     *   The 0-based index of the current repetition
     *
     * @param total
     *   The total number of repetitions
     */
    virtual void run_r_repetition(unsigned cur, unsigned total);

    /**
     * Executes the expansion of \a code, which has been put on top of the
     * opcode stack.
     *
     * By default it just calls run(), but it can be overridden to execute
     * operations before and after.
     *
     * @param code
     *   The D code that is being run
     */
    virtual void run_d_expansion(Varcode code);

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
    virtual void define_bitmap(unsigned bitmap_size);

    /**
     * Request processing, according to \a info, of a data variable.
     */
    virtual void define_variable(Varinfo info);

    /**
     * Request processing, according to \a info, of a data variable.
     */
    virtual void define_variable_with_associated_field(Varinfo info);

    /**
     * Request processing, according to \a info, of a data variabile that is
     * significant for controlling the encoding process.
     *
     * This means that the variable has always the same value on all datasets
     * (in case of compressed datasets), and that the interpreter needs to know
     * its value.
     *
     * @returns the value of the variable, or 0xffffffff if it is unset
     */
    virtual unsigned define_delayed_replication_factor(Varinfo info);

    /**
     * Request processing of a delayed replication factor variable used to
     * encode the size of a bitmap.
     *
     * @returns the repetition count
     */
    virtual unsigned define_bitmap_delayed_replication_factor(Varinfo info);

    /**
     * Request processing of an associated field significance variable
     * (B31021).
     *
     * @returns the associated field significance value
     */
    virtual unsigned define_associated_field_significance(Varinfo info);

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
 * Interpreter that pretty-prints the opcodes using indentation to show
 * structure
 */
class Printer : public Interpreter
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
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) override;
    void run_d_expansion(Varcode code) override;
    void define_variable(Varinfo info) override;
    void define_variable_with_associated_field(Varinfo info) override;
    void define_bitmap(unsigned bitmap_size) override;
    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;
};


}
}
#endif
