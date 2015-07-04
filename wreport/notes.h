#ifndef WREPORT_NOTES_H
#define WREPORT_NOTES_H

#include <iosfwd>

#ifndef WREPORT_PRINTF_ATTRS
#define WREPORT_PRINTF_ATTRS(a, b) __attribute__ ((format(printf, a, b)))
#endif

namespace wreport {

/**
 * Collect notes about unusual things that happen during processing.
 *
 * By default notes are discarded, unless set_target() is called or a
 * notes::Collect object is instantiated to direct notes where needed.
 */
namespace notes {

/// Set the target stream where the notes are sent
void set_target(std::ostream& out);

/// Get the current target stream for notes
std::ostream* get_target();

/// Return true if there is any target to which notes are sent
bool logs() throw ();

/// Output stream to send notes to
std::ostream& log() throw ();

/// printf-style logging
void logf(const char* fmt, ...) WREPORT_PRINTF_ATTRS(1, 2);

/**
 * RAII way to temporarily set a notes target.
 *
 * Notes are sent to the given output stream for as long as the object is in
 * scope.
 */
struct Collect
{
    /**
     * Old target stream to be restored whemn the object goes out of scope
     */
    std::ostream* old;

    /// Direct notes to \a out for the lifetime of the object
    Collect(std::ostream& out)
    {
        old = get_target();
        set_target(out);
    }
    ~Collect()
    {
        set_target(*old);
    }
};

}
}

#endif
