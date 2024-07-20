#ifndef WREPORT_TABLEDIR_H
#define WREPORT_TABLEDIR_H

#include <wreport/tableinfo.h>
#include <filesystem>
#include <string>
#include <vector>

namespace wreport {
struct Vartable;
struct DTable;

namespace tabledir {
struct Index;

struct Table
{
    std::string btable_id;
    std::filesystem::path btable_pathname;
    std::string dtable_id;
    std::filesystem::path dtable_pathname;

    Table(const std::filesystem::path& dirname, const std::string& filename);
    virtual ~Table() {}

    virtual void print_id(FILE* out) const;
};

/// Information about a version of a BUFR table
struct BufrTable : Table
{
    BufrTableID id;

    BufrTable(const BufrTableID& id, const std::string& dirname, const std::string& filename)
        : Table(dirname, filename), id(id) {}

    void print_id(FILE* out) const override;
};

/// Information about a version of a CREX table
struct CrexTable : Table
{
    CrexTableID id;

    CrexTable(const CrexTableID& id, const std::string& dirname, const std::string& filename)
        : Table(dirname, filename), id(id) {}

    void print_id(FILE* out) const override;
};


/// Indexed version of a table directory
struct Dir
{
    std::string pathname;
    time_t mtime;
    std::vector<Table*> tables;

    Dir(const std::string& pathname);
    Dir(const Dir&) = delete;
    Dir(Dir&&) = default;
    ~Dir();

    Dir& operator=(const Dir&) = delete;

    /// Reread the directory contents if it has changed
    void refresh();
};

class Tabledirs
{
protected:
    std::vector<std::string> dirs;
    Index* index;

public:
    Tabledirs();
    Tabledirs(const Tabledirs&) = delete;
    ~Tabledirs();

    Tabledirs& operator=(const Tabledirs&) = delete;

    /**
     * Add the default directories according to compile-time and environment
     * variables.
     */
    void add_default_directories();

    /// Add a table directory to this collection
    void add_directory(const std::string& dir);

    /// Find a BUFR table
    const tabledir::Table* find_bufr(const BufrTableID& id);

    /// Find a CREX table
    const tabledir::Table* find_crex(const CrexTableID& id);

    /// Find a BUFR or CREX table by file name
    const tabledir::Table* find(const std::string& basename);

    /// Print a list of all tables found
    void print(FILE* out);

    /// Print the step by step process by which a table is selected for \a id
    void explain_find_bufr(const BufrTableID& id, FILE* out);

    /// Print the step by step process by which a table is selected for \a id
    void explain_find_crex(const CrexTableID& id, FILE* out);

    /// Get the default tabledir instance
    static Tabledirs& get();
};

}
}

#endif
