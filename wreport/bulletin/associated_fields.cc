#include "associated_fields.h"
#include "wreport/var.h"
#include "wreport/vartable.h"
#include "wreport/notes.h"

using namespace std;

namespace wreport {
namespace bulletin {

// Return a value with bitlen bits set to 1
static inline uint32_t all_ones(int bitlen)
{
    return ((1 << (bitlen - 1))-1) | (1 << (bitlen - 1));
}

AssociatedField::AssociatedField(const Vartable& btable)
    : btable(btable), skip_missing(true), bit_count(0), significance(63)
{
}
AssociatedField::~AssociatedField() {}

std::unique_ptr<Var> AssociatedField::make_attribute(unsigned value) const
{
    bool missing = value == all_ones(bit_count);
    switch (significance)
    {
        case 1:
            // Add attribute B33002=value
            if (missing)
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 2))));
            else
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 2)), (int)value));
        case 2:
            // Add attribute B33003=value
            if (missing)
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 3))));
            else
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 3)), (int)value));
        case 3:
        case 4:
        case 5:
            // Reserved: ignored
            notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            return unique_ptr<Var>();
        case 6:
            // Add attribute B33050=value
            if (missing)
            {
                if (skip_missing)
                    return std::unique_ptr<Var>();
                else
                    return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 50))));
            } else
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 50)), (int)value));
        case 7:
            // Add attribute B33040=value
            if (missing)
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 40))));
            else
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 40)), (int)value));
        case 8:
            // Add attribute B33002=value
            if (missing)
            {
                if (skip_missing)
                    return unique_ptr<Var>();
                else
                    return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 2))));
            } else
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 2)), (int)value));
        case 21:
            // Add attribute B33041=value
            if (!skip_missing || !missing)
                return std::unique_ptr<Var>(new Var(btable.query(WR_VAR(0, 33, 41)), 0));
            else
                return std::unique_ptr<Var>();
        case 63:
            /*
             * Ignore quality information if B31021 is missing.
             * The Guide to FM94-BUFR says:
             *   If the quality information has no meaning for some
             *   of those following elements, but the field is
             *   still there, there is at present no explicit way
             *   to indicate "no meaning" within the currently
             *   defined meanings. One must either redefine the
             *   meaning of the associated field in its entirety
             *   (by including 0 31 021 in the message with a data
             *   value of 63 - "missing value") or remove the
             *   associated field bits by the "cancel" operator: 2
             *   04 000.
             */
            return std::unique_ptr<Var>();
        default:
            if (significance >= 9 and significance <= 20)
                // Reserved: ignored
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            else if (significance >= 22 and significance <= 62)
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
                        significance);
            else
                error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", significance);
            return std::unique_ptr<Var>();
    }
}

const Var* AssociatedField::get_attribute(const Var& var) const
{
    /*
     * Query variable attribute according to significance given in CODE TABLE
     * 031021
     */
    switch (significance)
    {
        case 1:
        case 8:
            return var.enqa(WR_VAR(0, 33, 2));
            break;
        case 2:
            return var.enqa(WR_VAR(0, 33, 3));
            break;
        case 3:
        case 4:
        case 5:
            // Reserved: ignored
            notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            break;
        case 6: return var.enqa(WR_VAR(0, 33, 50)); break;
        case 7: return var.enqa(WR_VAR(0, 33, 40)); break;
        case 21: return var.enqa(WR_VAR(0, 33, 41)); break;
        case 63:
            /*
             * Ignore quality information if B31021 is missing.
             * The Guide to FM94-BUFR says:
             *   If the quality information has no meaning for some
             *   of those following elements, but the field is
             *   still there, there is at present no explicit way
             *   to indicate "no meaning" within the currently
             *   defined meanings. One must either redefine the
             *   meaning of the associated field in its entirety
             *   (by including 0 31 021 in the message with a data
             *   value of 63 - "missing value") or remove the
             *   associated field bits by the "cancel" operator: 2
             *   04 000.
             */
            break;
        default:
            if (significance >= 9 and significance <= 20)
                // Reserved: ignored
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved'\n",
                    significance);
            else if (significance >= 22 and significance <= 62)
                notes::logf("Ignoring B31021=%d, which is documented as 'reserved for local use'\n",
                        significance);
            else
                error_unimplemented::throwf("C04 modifiers with B31021=%d are not supported", significance);
            break;
    }
    return 0;
}

}
}
