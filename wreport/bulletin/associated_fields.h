#ifndef WREPORT_BULLETIN_ASSOCIATED_FIELDS_H
#define WREPORT_BULLETIN_ASSOCIATED_FIELDS_H

#include <memory>

namespace wreport {
struct Var;
struct Vartable;

namespace bulletin {

struct AssociatedField
{
    /// B table used to generate associated field attributes
    const Vartable& btable;

    /**
     * If true, fields with a missing values will be returned as 0. If it is
     * false, fields with missing values will be returned as undefined
     * variables.
     */
    bool skip_missing;

    /**
     * Number of extra bits inserted by the current C04yyy modifier (0 for no
     * C04yyy operator in use)
     */
    unsigned bit_count;

    /// Significance of C04yyy field according to code table B31021
    unsigned significance;


    AssociatedField(const Vartable& btable);
    ~AssociatedField();

    /**
     * Create a Var that can be used as an attribute for the currently defined
     * associated field and the given value.
     *
     * A return value of nullptr means "no field to associate".
     *
     */
    std::unique_ptr<Var> make_attribute(unsigned value) const;

    /**
     * Get the attribute of var corresponding to this associated field
     * significance.
     */
    const Var* get_attribute(const Var& var) const;
};

}
}
#endif
