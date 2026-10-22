#ifndef WREPORT_FILES_H
#define WREPORT_FILES_H

#include <cstddef>
#include <string>

namespace wreport {

/**
 * Generic binary input file interface
 */
class InputFile
{
public:
    virtual ~InputFile() {}

    /// Return the file name, if known
    virtual const std::string& name() const = 0;

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
     * @returns the offset in the file of the beginning of the signature, or -1
     *   for end of file
     */
    virtual off_t seek_past_signature(const char* sig, unsigned sig_len) = 0;

    /**
     * Read \a size bytes from the file into the given memory buffer.
     *
     * @param[what] description of the operation, used to format error messages
     */
    virtual void read(char* buf, size_t size, const char* what) = 0;

    /**
     * Read bytes into \a out until the CREX terminator is found
     */
    virtual void read_until_crex_terminator(std::string& out) = 0;
};

/**
 *  input file using stdio primitives.
 */
class InputFileStdio : public InputFile
{
protected:
    /// Stdio file descriptor
    FILE* fd;
    /// Current offset in the file
    off_t offset;
    /// File name (used for error messages)
    std::string m_name;

public:
    explicit InputFileStdio(FILE* fd, off_t start_offset,
                            const std::string& name = std::string())
        : fd(fd), offset(start_offset), m_name(name)
    {
    }
    InputFileStdio(const InputFileStdio&) = delete;
    InputFileStdio(InputFileStdio&& o)
        : fd(o.fd), offset(o.offset), m_name(std::move(o.m_name))
    {
        o.fd = nullptr;
    }
    InputFileStdio& operator=(const InputFileStdio&) = delete;
    InputFileStdio& operator=(InputFileStdio&& o)
    {
        if (this != &o)
        {
            fd     = o.fd;
            o.fd   = nullptr;
            offset = o.offset;
            m_name = std::move(o.m_name);
        }
        return *this;
    }

    const std::string& name() const override { return m_name; }
    off_t seek_past_signature(const char* sig, unsigned sig_len) override;
    void read(char* buf, size_t size, const char* what) override;
    void read_until_crex_terminator(std::string& out) override;
};

} // namespace wreport

#endif
