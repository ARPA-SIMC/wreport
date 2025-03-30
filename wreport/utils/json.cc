#import "json.h"
#include <wreport/error.h>

namespace wreport::json {
JSON::JSON(std::stringstream& out) : out(out) {}

void JSON::addc(char c)
{
    out.put(c);
    if (out.bad())
        error_system::throwf("write to buffer failed (one char)");
}
void JSON::adds(const std::string& val)
{
    out << val;
    if (out.bad())
        error_system::throwf("write to buffer failed (one %zu-bytes string)",
                             val.size());
}
void JSON::addu(unsigned val)
{
    out << val;
    if (out.bad())
        error_system::throwf("write to buffer failed (one unsigned)");
}
void JSON::addq(const std::string& val)
{
    addc('"');
    for (const auto& c : val)
        switch (c)
        {
            case '"':  adds("\\\""); break;
            case '\\': adds("\\\\"); break;
            case '/':  adds("\\/"); break;
            case '\b': adds("\\b"); break;
            case '\f': adds("\\f"); break;
            case '\n': adds("\\n"); break;
            case '\r': adds("\\r"); break;
            case '\t': adds("\\t"); break;
            default:   addc(c); break;
        }
    addc('"');
}

JSONL::JSONL(std::stringstream& out) : JSON(out) {}
JSONL::~JSONL() { addc('\n'); }
Dict JSONL::dict() { return Dict(out); }

void Sequence::maybe_add_comma()
{
    if (first)
        first = false;
    else
        addc(',');
}

Dict::Dict(std::stringstream& out) : Sequence(out) { addc('{'); }
Dict::~Dict() { addc('}'); }

void Dict::add_null(const char* key)
{
    maybe_add_comma();
    addq(key);
    adds(": null");
}
void Dict::add_bool(const char* key, bool val)
{
    maybe_add_comma();
    addq(key);
    if (val)
        adds(": true");
    else
        adds(": false");
}
void Dict::add(const char* key, const std::string& val)
{
    maybe_add_comma();
    addq(key);
    addc(':');
    addq(val);
}
void Dict::add_unsigned(const char* key, unsigned val)
{
    maybe_add_comma();
    addq(key);
    addc(':');
    addu(val);
}
void Dict::add_nullable(const char* key, uint8_t val, uint8_t nullval)
{
    if (val == nullval)
        add_null(key);
    else
        add_unsigned(key, val);
}
void Dict::add_nullable(const char* key, uint16_t val, uint16_t nullval)
{
    if (val == nullval)
        add_null(key);
    else
        add_unsigned(key, val);
}
Dict Dict::add_dict(const char* key)
{
    maybe_add_comma();
    addq(key);
    addc(':');
    return Dict(out);
}
List Dict::add_list(const char* key)
{
    maybe_add_comma();
    addq(key);
    addc(':');
    return List(out);
}

List::List(std::stringstream& out) : Sequence(out) { addc('['); }
List::~List() { addc(']'); }

List List::add_list()
{
    maybe_add_comma();
    return List(out);
}
Dict List::add_dict()
{
    maybe_add_comma();
    return Dict(out);
}

} // namespace wreport::json
