#include "files.h"
#include "error.h"
#include <cerrno>
#include <cstdio>

namespace wreport {

off_t InputFileStdio::seek_past_signature(const char* sig, unsigned sig_len)
{
    unsigned got = 0;
    int c;

    errno = 0;

    while (got < sig_len && (c = getc(fd)) != EOF)
    {
        ++offset;
        if (c == sig[got])
            got++;
        else
            got = 0;
    }

    if (errno != 0)
    {
        if (m_name.empty())
            error_system::throwf("looking for start of %.4s data", sig);
        else
            error_system::throwf("looking for start of %.4s data in %s:", sig,
                                 m_name.c_str());
    }

    if (got != sig_len)
    {
        // End of file: return accordingly
        return -1;
    }
    return offset - sig_len;
}

void InputFileStdio::read(char* buf, size_t size, const char* what)
{
    if (fread(buf, size, 1, fd) != 1)
    {
        if (ferror(fd))
        {
            if (m_name.empty())
                error_system::throwf("cannot read %s (%zd bytes)", what, size);
            else
                error_system::throwf("cannot read %s (%zd bytes) from %s", what,
                                     size, m_name.c_str());
        }
        else if (feof(fd))
        {
            if (m_name.empty())
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) (premature end of file)", what,
                    size);
            else
                error_consistency::throwf("cannot read %s (%zd bytes) from %s "
                                          "(premature end of file)",
                                          what, size, m_name.c_str());
        }
        else
        {
            if (m_name.empty())
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) (partial read)", what, size);
            else
                error_consistency::throwf(
                    "cannot read %s (%zd bytes) from %s (partial read)", what,
                    size, m_name.c_str());
        }
    }
    offset += size;
}

void InputFileStdio::read_until_crex_terminator(std::string& out)
{
    // Read until "\+\+(\r|\n)+7777"
    {
        const char* target           = "++\r\n7777";
        static const int target_size = 8;
        int got                      = 0;
        int c;

        errno = 0;
        while (got < 8 && (c = getc(fd)) != EOF)
        {
            if (target[got] == '\r' && (c == '\n' || c == '\r'))
                got++;
            else if (target[got] == '\n' && (c == '\n' || c == '\r'))
                ;
            else if (target[got] == '\n' && c == '7')
                got += 2;
            else if (c == target[got])
                got++;
            else
                got = 0;

            out += (char)c;
        }

        if (ferror(fd))
        {
            if (m_name.empty())
                throw error_system("cannot find end of CREX data");
            else
                error_system::throwf("cannot find end of CREX data in %s",
                                     m_name.c_str());
        }

        if (got != target_size)
        {
            if (m_name.empty())
                throw error_consistency("CREX message is incomplete");
            else
                error_consistency::throwf("CREX message is incomplete in %s",
                                          m_name.c_str());
        }
    }
}

} // namespace wreport
