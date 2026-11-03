#ifndef WREPORT_PYTHON_FILES_H
#define WREPORT_PYTHON_FILES_H

#include "utils/core.h"
#include <wreport/files.h>
#include <wreport/python.h>

namespace wreport ::python {

class InputFilePython : public wreport::InputFile
{
protected:
    /// Python file or file-like object
    PyObject* obj;
    /// Current offset in the file
    off_t offset;
    /// Cached file name computed by name()
    mutable std::string cached_name;

    /**
     * Read the next character from the file.
     *
     * @returns the character or EOF on end of file
     */
    char getc(const char* what);

public:
    explicit InputFilePython(PyObject* obj);
    InputFilePython(const InputFilePython&) = delete;
    InputFilePython(InputFilePython&& o) : obj(o.obj), offset(o.offset)
    {
        o.obj = nullptr;
    }
    InputFilePython& operator=(const InputFilePython&) = delete;
    InputFilePython& operator=(InputFilePython&& o)
    {
        if (this != &o)
        {
            obj    = o.obj;
            o.obj  = nullptr;
            offset = o.offset;
        }
        return *this;
    }
    virtual ~InputFilePython();

    /// Return the file name, if known
    const std::string& name() const override;

    /**
     * Seek forward looking for a signature.
     *
     * If the signature is found, the file pointer is left to the first byte
     * after the signature, and the function returns true.
     *
     * If the signature is not found, the file pointer is left at the end of
     * the file, and the function returns false.
     *
     * @param[sig] buffer with the signature bytes
     * @param[sig_len] length of the signature in bytes
     * @returns the offset in the file of the beginning of the signature, or
     * -1 for end of file
     */
    off_t seek_past_signature(const char* sig, unsigned sig_len) override;

    /**
     * Read \a size bytes from the file into the given memory buffer.
     *
     * @param[what] description of the operation, used to format error
     * messages
     */
    void read(char* buf, size_t size, const char* what) override;

    /**
     * Read bytes into \a out until the CREX terminator is found
     */
    void read_until_crex_terminator(std::string& out) override;
};

} // namespace wreport::python

#endif
