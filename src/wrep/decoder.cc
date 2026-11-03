#include "decoder.h"

void DecodeHeader::handle_raw_bufr(const std::string& raw_data,
                                   const char* fname, long offset)
{
    try
    {
        // Decode the raw data. fname and offset are optional and we pass
        // them just to have nicer error messages
        auto bulletin =
            wreport::BufrBulletin::decode_header(raw_data, fname, offset);

        // Do something with the decoded information
        handle_bufr(*bulletin);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s:%ld:%s\n", fname, offset, e.what());
    }
}

void DecodeHeader::handle_raw_crex(const std::string& raw_data,
                                   const char* fname, long offset)
{
    try
    {
        // Decode the raw data. fname and offset are optional and we pass
        // them just to have nicer error messages
        auto bulletin = wreport::CrexBulletin::decode(raw_data, fname, offset);

        // Do something with the decoded information
        handle_crex(*bulletin);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s:%ld:%s\n", fname, offset, e.what());
    }
}

void DecodeFull::handle_raw_bufr(const std::string& raw_data, const char* fname,
                                 long offset)
{
    try
    {
        // Decode the raw data. fname and offset are optional and we pass
        // them just to have nicer error messages
        auto bulletin = wreport::BufrBulletin::decode(raw_data, fname, offset);

        // Do something with the decoded information
        handle_bufr(*bulletin);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s:%ld:%s\n", fname, offset, e.what());
    }
}

void DecodeFull::handle_raw_crex(const std::string& raw_data, const char* fname,
                                 long offset)
{
    try
    {
        // Decode the raw data. fname and offset are optional and we pass
        // them just to have nicer error messages
        auto bulletin = wreport::CrexBulletin::decode(raw_data, fname, offset);

        // Do something with the decoded information
        handle_crex(*bulletin);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s:%ld:%s\n", fname, offset, e.what());
    }
}
