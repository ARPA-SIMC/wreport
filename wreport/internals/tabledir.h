#ifndef WREPORT_TABLEDIR_H
#define WREPORT_TABLEDIR_H

#include <string>
#include <vector>

namespace wreport {
struct Vartable;
struct DTable;

namespace tabledir {
struct Index;

struct Table
{
    const Vartable* btable;
    std::string btable_id;
    std::string btable_pathname;
    const DTable* dtable;
    std::string dtable_id;
    std::string dtable_pathname;

    Table(const std::string& dirname, const std::string& filename);
    virtual ~Table() {}

    /// Load btable and dtable if they have not been loaded yet
    virtual void load_if_needed() = 0;
};

/// Information about a version of a BUFR table
struct BufrTable : Table
{
    int centre;
    int subcentre;
    int master_table;
    int local_table;

    BufrTable(int centre, int subcentre, int master_table, int local_table,
              const std::string& dirname, const std::string& filename)
        : Table(dirname, filename),
          centre(centre), subcentre(subcentre),
          master_table(master_table), local_table(local_table) {}

    void load_if_needed() override;
};

/// Information about a version of a CREX table
struct CrexTable : Table
{
    int master_table_number;
    int edition;
    int table;

    CrexTable(int master_table_number, int edition, int table,
              const std::string& dirname, const std::string& filename)
        : Table(dirname, filename), master_table_number(master_table_number),
          edition(edition), table(table) {}

    void load_if_needed() override;
};


/// Indexed version of a table directory
struct Dir
{
    std::string pathname;
    time_t mtime;
    std::vector<BufrTable> bufr_tables;
    std::vector<CrexTable> crex_tables;

    Dir(const std::string& pathname);
    ~Dir();

    /// Reread the directory contents if it has changed
    void refresh();
};

/// Query for a BUFR table
struct BufrQuery
{
    int centre;
    int subcentre;
    int master_table;
    int local_table;
    BufrTable* result;

    BufrQuery(int centre, int subcentre, int master_table, int local_table);
    void search(Dir& dir);
    bool is_better(const BufrTable& t);
};

/// Query for a CREX table
struct CrexQuery
{
    int master_table_number;
    int edition;
    int table;
    CrexTable* result;

    CrexQuery(int master_table_number, int edition, int table);
    void search(Dir& dir);
    bool is_better(const CrexTable& t);
};

class Tabledir
{
protected:
    std::vector<std::string> dirs;
    Index* index;

public:
    Tabledir();
    ~Tabledir();

    /**
     * Add the default directories according to compile-time and environment
     * variables.
     */
    void add_default_directories();

    /// Add a table directory to this collection
    void add_directory(const std::string& dir);

    /// Find a BUFR table
    const tabledir::BufrTable* find_bufr(int centre, int subcentre, int master_table, int local_table);

    /// Find a CREX table
    const tabledir::CrexTable* find_crex(int master_table_number, int edition, int table);

    /// Find a BUFR table by file name
    const tabledir::BufrTable* find_bufr(const std::string& basename);

    /// Find a CREX table by file name
    const tabledir::CrexTable* find_crex(const std::string& basename);

    /// Get the default tabledir instance
    static Tabledir& get();
};

}
}

#endif
