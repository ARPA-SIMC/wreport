#include "fs.h"
#include "error.h"
#include <cstdlib>
#include <cstddef>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

namespace wreport {
namespace fs {

Directory::Directory(const std::string& pathname)
    : pathname(pathname), fd(-1)
{
    fd = open(pathname.c_str(), O_DIRECTORY | O_PATH);
    if (fd == -1)
    {
       if (errno != ENOENT)
           error_system::throwf("cannot open directory %s", pathname.c_str());
       else
           return;
    }
}

Directory::~Directory()
{
    if (fd != -1)
        close(fd);
}

void Directory::stat(struct stat& st)
{
    if (fstat(fd, &st) == -1)
        error_system::throwf("cannot stat directory %s", pathname.c_str());
}

Directory::const_iterator::const_iterator()
{
}

Directory::const_iterator::const_iterator(const Directory& dir)
{
    // If the directory does not exist, just remain an end iterator
    if (!dir.exists()) return;

    int fd1 = openat(dir.fd, ".", O_DIRECTORY);
    if (fd1 == -1)
        error_system::throwf("cannot open directory %s", dir.pathname.c_str());

    this->dir = fdopendir(fd1);
    if (!this->dir)
        error_system::throwf("opendir failed on directory %s", dir.pathname.c_str());

    long name_max = fpathconf(dir.fd, _PC_NAME_MAX);
    if (name_max == -1) // Limit not defined, or error: take a guess
        name_max = 255;
    size_t len = offsetof(dirent, d_name) + name_max + 1;
    cur_entry = (struct dirent*)malloc(len);
    if (cur_entry == NULL)
        throw error_alloc("cannot allocate space for a dirent structure");

    operator++();
}

Directory::const_iterator::~const_iterator()
{
    if (cur_entry) free(cur_entry);
    if (dir) closedir(dir);
}

bool Directory::const_iterator::operator==(const const_iterator& i) const
{
    if (!dir && !i.dir) return true;
    if (!dir || !i.dir) return false;
    return cur_entry->d_ino == i.cur_entry->d_ino;
}
bool Directory::const_iterator::operator!=(const const_iterator& i) const
{
    if (!dir && !i.dir) return false;
    if (!dir || !i.dir) return true;
    return cur_entry->d_ino != i.cur_entry->d_ino;
}

void Directory::const_iterator::operator++()
{
    struct dirent* result;
    if (readdir_r(dir, cur_entry, &result) != 0)
        error_system::throwf("readdir_r failed");

    if (result == nullptr)
    {
        // Turn into an end iterator
        free(cur_entry);
        cur_entry = nullptr;
        closedir(dir);
        dir = nullptr;
    }
}

}
}
