#ifndef WREPORT_UTILS_JSON_H
#define WREPORT_UTILS_JSON_H

/**
 * Limited JSON output used for tests
 */

#include <cstdint>
#include <sstream>

namespace wreport::json {
class Dict;
class List;

class JSON
{
protected:
    std::stringstream& out;

    void addc(char c);
    void adds(const std::string& val);
    void addu(unsigned val);
    void addq(const std::string& val);

public:
    explicit JSON(std::stringstream& out);
};

class JSONL : public JSON
{
public:
    explicit JSONL(std::stringstream& out);
    ~JSONL();

    Dict dict();
};

class Sequence : public JSON
{
protected:
    bool first = true;

    void maybe_add_comma();

public:
    using JSON::JSON;
};

class Dict : public Sequence
{
public:
    explicit Dict(std::stringstream& out);
    ~Dict();

    void add_null(const char* key);
    void add_bool(const char* key, bool val);
    void add(const char* key, const std::string& val);
    void add_unsigned(const char* key, unsigned val);
    void add_nullable(const char* key, uint8_t val, uint8_t nullval);
    void add_nullable(const char* key, uint16_t val, uint16_t nullval);
    Dict add_dict(const char* key);
    List add_list(const char* key);
};

class List : public Sequence
{
public:
    explicit List(std::stringstream& out);
    ~List();

    Dict add_dict();
    List add_list();
};

} // namespace wreport::json

#endif
