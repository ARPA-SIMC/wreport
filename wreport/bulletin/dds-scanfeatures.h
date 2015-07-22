#ifndef WREPORT_BULLETIN_DDS_SCANFEATURES_H
#define WREPORT_BULLETIN_DDS_SCANFEATURES_H

#include <wreport/bulletin/interpreter.h>
#include <set>

namespace wreport {
namespace bulletin {

/**
 * Interpreter that scans what features are used by a bulletin
 */
class ScanFeatures : public Interpreter
{
public:
    /// Features that have been found
    std::set<std::string> features;

    ScanFeatures(const Tables& tables, const Opcodes& opcodes);

    void c_modifier(Varcode code, Opcodes& next) override;
    void r_replication(Varcode code, Varcode delayed_code, const Opcodes& ops) override;
    void define_variable(Varinfo info) override;
    void define_bitmap(unsigned bitmap_size) override;
    unsigned define_associated_field_significance(Varinfo info) override;
    unsigned define_bitmap_delayed_replication_factor(Varinfo info) override;
};

}
}
#endif
