#ifndef WREPORT_INTERNALS_FS_H
#define WREPORT_INTERNALS_FS_H

#include <string>
#include <iterator>
#include <dirent.h>
#include <sys/stat.h>

namespace wreport {
namespace fs {

/**
 * Access a directory on the file system
 */
struct Directory
{
    /// Pathname of the directory
    const std::string& pathname;

    /**
     * O_PATH file descriptor pointing at the directory, or -1 if the
     * directory does not exist
     */
    int fd;

    /**
     * Iterator for directory entries
     */
    struct const_iterator : public std::iterator<std::input_iterator_tag, struct dirent>
    {
        DIR* dir = 0;
        struct dirent* cur_entry = 0;

        // End iterator
        const_iterator();
        // Start iteration on dir
        const_iterator(const Directory& dir);
        const_iterator(const const_iterator&) = delete;
        const_iterator(const_iterator&& o)
            : dir(o.dir), cur_entry(o.cur_entry)
        {
            o.dir = nullptr;
            o.cur_entry = nullptr;
        }
        ~const_iterator();
        const_iterator& operator=(const const_iterator&) = delete;
        const_iterator& operator=(const_iterator&&) = delete;

        bool operator==(const const_iterator& i) const;
        bool operator!=(const const_iterator& i) const;
        struct dirent& operator*() const { return *cur_entry; }
        struct dirent* operator->() const { return cur_entry; }
        void operator++();
    };

    Directory(const std::string& pathname);
    ~Directory();

    /// Begin iterator on all directory entries
    const_iterator begin() const { return const_iterator(*this); }

    /// End iterator on all directory entries
    const_iterator end() const { return const_iterator(); }

    /// Check if the directory exists
    bool exists() const { return fd != -1; }

    /// Call stat(2) on the directory
    void stat(struct stat& st);
};

}
}

#endif
